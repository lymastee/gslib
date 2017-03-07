/*
 * Copyright (c) 2016-2017 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sys/timeb.h>
#include <time.h>
#include <gslib/uuid.h>
#include <gslib/md5.h>
#include <gslib/sha1.h>
#include <gslib/mtrand.h>

#ifdef _WIN32
#include <snmp.h>
#endif

__gslib_begin__

uuid::uuid()
{
    auto* raw = static_cast<uuid_raw*>(this);
    memset(raw, 0, sizeof(uuid_raw));
}

bool uuid::is_valid() const
{
    if(!data1)
        return false;
    /* check version bits */
    auto ver_bits = data3 >> 12;
    return ver_bits >= 1 && ver_bits <= 5;
}

static void write_uint16(byte* stream, uint16 data)
{
    assert(stream);
    stream[0] = (byte)(data >> 8);
    stream[1] = (byte)data;
}

static void write_uint(byte* stream, uint data)
{
    assert(stream);
    stream[0] = (byte)(data >> 24);
    stream[1] = (byte)(data >> 16);
    stream[2] = (byte)(data >> 8);
    stream[3] = (byte)data;
}

static void read_uint16(uint16* buf, const byte* stream)
{
    assert(buf && stream);
    *buf = ((uint16)stream[0]) << 8 | stream[1];
}

static void read_uint(uint* buf, const byte* stream)
{
    assert(buf && stream);
    *buf = (((uint)stream[0]) << 24) |
        (((uint)stream[1]) << 16) |
        (((uint)stream[2]) << 8) |
        stream[3];
}

/*
 * The announcement of _newnode calling was like:
 * new_node(uint16& node_hi, uint& node_lo) -> uint16 ["version mask"]
 */
template<class _newnode>
static void generate_uuid(uuid_raw* uid, _newnode new_node)
{
    assert(uid);
    __timeb64 tb;
    _ftime64_s(&tb);
    /* counter of 100ns time interval since oct.15, 1582 */
    uint64 time = ((uint64)tb.time) * 1000000 + ((uint64)tb.millitm) * 1000;
    time = time * 10 + 0x01b21dd213814000;
    uint16 mask = new_node(uid->data4 + 2);
    /*
     * about the clock seq, we won't use a seq counter actually, a 16 bit counter
     * with a random start was useless to avoid duplicates.
     * so we simply use a random 16 bit number to replace it.
     */
    uint16 clockseq = (uint16)mtrand();
    clockseq = (clockseq & 0x3fff) | 0x8000;    /* ahead of it was binary 1 0 */
    /* stuffing */
    uid->data1 = (uint)time;
    uid->data2 = (uint16)(time >> 32);
    uid->data3 = (uint16)((time >> 48) & 0x0fff | mask);
    write_uint16(uid->data4, (uint16)((clockseq & 0x3fff) | 0x8000));
}

#ifdef _WIN32

typedef bool(__stdcall * FnSnmpExtensionInit)(
    DWORD dwTimeZeroReference,
    HANDLE* hPollForTrapEvent,
    AsnObjectIdentifier* supportedView
    );

typedef bool(__stdcall * FnSnmpExtensionTrap)(
    AsnObjectIdentifier* exterprise,
    AsnInteger* genericTrap,
    AsnTimeticks* timeStamp,
    RFC1157VarBindList* variableBindings
    );

typedef bool(__stdcall * FnSnmpExtensionQuery)(
    BYTE requestType,
    RFC1157VarBindList* variableBindings,
    AsnInteger* errorStatus,
    AsnInteger* errorIndex
    );

typedef bool(__stdcall * FnSnmpExtensionInitEx)(
    AsnObjectIdentifier* supportedView
    );

#pragma comment(lib, "Snmpapi.lib")

static bool get_valid_mac_address(byte node[6])
{
    static BYTE mac_addr[6];
    static bool has_mac_addr = false;
    if(!has_mac_addr) {
        HINSTANCE hInst;
        FnSnmpExtensionInit fnInit = 0;
        FnSnmpExtensionInitEx fnInitEx = 0;
        FnSnmpExtensionQuery fnQuery = 0;
        FnSnmpExtensionTrap fnTrap = 0;
        HANDLE pollForTrapEvent;
        AsnObjectIdentifier supportedView;
        UINT OID_ifEntryType[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 3 };
        UINT OID_ifEntryNum[] = { 1, 3, 6, 1, 2, 1, 2, 1 };
        UINT OID_ipMacEntAddr[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 6 };
        AsnObjectIdentifier MIB_ifMacEntAddr = { _countof(OID_ipMacEntAddr), OID_ipMacEntAddr };
        AsnObjectIdentifier MIB_ifEntryType = { _countof(OID_ifEntryType), OID_ifEntryType };
        AsnObjectIdentifier MIB_ifEntryNum = { _countof(OID_ifEntryNum), OID_ifEntryNum };
        RFC1157VarBindList varBindList;
        RFC1157VarBind varBind[2];
        AsnInteger errorStatus;
        AsnInteger errorIndex;
        AsnObjectIdentifier MIB_NULL = { 0, 0 };
        int ret;
        int dtmp;
        int i = 0, j = 0;
        bool found = false;
        //char tempEthernet[13];
        hInst = LoadLibrary(_t("inetmib1.dll"));
        if(hInst < (HINSTANCE)HINSTANCE_ERROR) {
            assert(!"load library failed: \"inetmib1.dll\"");
            hInst = 0;
            return false;
        }
        fnInit = (FnSnmpExtensionInit)GetProcAddress(hInst, "SnmpExtensionInit");
        fnInitEx = (FnSnmpExtensionInitEx)GetProcAddress(hInst, "SnmpExtensionInitEx");
        fnQuery = (FnSnmpExtensionQuery)GetProcAddress(hInst, "SnmpExtensionQuery");
        fnTrap = (FnSnmpExtensionTrap)GetProcAddress(hInst, "SnmpExtensionTrap");
        fnInit(GetTickCount(), &pollForTrapEvent, &supportedView);
        varBindList.list = varBind;
        varBind[0].name = MIB_NULL;
        varBind[1].name = MIB_NULL;
        varBindList.len = 1;
        SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryNum);
        ret = fnQuery(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
        varBindList.len = 2;
        SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryType);
        SNMP_oidcpy(&varBind[1].name, &MIB_ifMacEntAddr);
        do {
            ret = fnQuery(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
            ret = !ret ? 1 : SNMP_oidncmp(&varBind[0].name, &MIB_ifEntryType, MIB_ifEntryType.idLength);
            if(!ret) {
                j ++;
                dtmp = varBind[0].value.asnValue.number;
                if(dtmp == 6) {
                    ret = SNMP_oidncmp(&varBind[1].name, &MIB_ifMacEntAddr, MIB_ifMacEntAddr.idLength);
                    if(!ret && varBind[1].value.asnValue.address.stream) {
                        static const BYTE DUN_adapter[] = { 0x44, 0x45, 0x53, 0x54, 0x00 };
                        if(memcmp(DUN_adapter, varBind[1].value.asnValue.address.stream, _countof(DUN_adapter)) == 0) {
                            /* This is a DUN adapter. */
                            continue;
                        }
                        static const BYTE NULL_adapter[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                        if(memcmp(NULL_adapter, varBind[1].value.asnValue.address.stream, _countof(NULL_adapter)) == 0) {
                            /* This is a NULL adapter. */
                            continue;
                        }
                        assert(varBind[1].value.asnValue.address.length == 6);
                        memcpy_s(mac_addr, sizeof(mac_addr), varBind[1].value.asnValue.address.stream, 6);
                        has_mac_addr = true;
                        break;
                    }
                }
            }
        }
        while(!ret);
        FreeLibrary(hInst);
        SNMP_FreeVarBind(&varBind[0]);
        SNMP_FreeVarBind(&varBind[1]);
        if(!has_mac_addr) {
            assert(!"get_valid_mac_address failed.");
            return false;
        }
    }
    assert(has_mac_addr);
    memcpy_s(node, 6, mac_addr, 6);
    return true;
}

#endif  /* _WIN32 */

static void generate_uuid_v1(uuid_raw* uid)
{
    assert(uid);
    return generate_uuid(uid, [](byte node[])-> uint16 {
        get_valid_mac_address(node);
        return 0x1000;
    });
}

static void generate_uuid_v2(uuid_raw* uid)
{
    assert(uid);
    assert(!"The DCE security version of UUID was invalid.");
}

static void generate_uuid_v4(uuid_raw* uid)
{
    assert(uid);
    return generate_uuid(uid, [](byte node[])-> uint16 {
        auto r = mtrand64();
        write_uint16(node, (uint16)((r >> 32) & 0xffff));
        write_uint(node + 2, (uint)(r & 0xffffffff));
        return 0x4000;
    });
}

void uuid::generate(uuid_version ver)
{
    switch(ver)
    {
    case uuid_v1:
        return generate_uuid_v1(this);
    case uuid_v2:
        return generate_uuid_v2(this);
    case uuid_v4:
        return generate_uuid_v4(this);
    default:
        assert(!"unexpected uuid version.");
        return;
    }
}

static void generate_uuid_from_name(uuid_raw* uid, const byte hash[16], uint16 maskv)
{
    assert(uid && hash);
    read_uint(&uid->data1, hash);
    read_uint16(&uid->data2, hash + 4);
    read_uint16(&uid->data3, hash + 6);
    memcpy_s(uid->data4, sizeof(uid->data4), hash + 8, 8);
    uid->data3 = ((uid->data3 & 0x0fff) | maskv);
    uid->data4[0] = ((uid->data4[0] & 0x3f) | 0x80);
}

static void generate_uuid_v3(uuid_raw* uid, const gchar* name, int len)
{
    assert(uid && name);
    _string<char> str;
    str.from(name, len);    /* convert to char */
    md5 encrypt(str.c_str(), str.length());
    generate_uuid_from_name(uid, encrypt.get_digest(), 0x3000);
}

static void generate_uuid_v5(uuid_raw* uid, const gchar* name, int len)
{
    assert(uid && name);
    _string<char> str;
    str.from(name, len);    /* convert to char */
    sha1 encrypt(str.c_str(), str.length());
    byte hshbuff[20];
    auto* d = encrypt.get_digest();
    assert(d);
    for(int i = 0; i < 5; i ++)
        write_uint(&hshbuff[i * 4], d[i]);
    generate_uuid_from_name(uid, hshbuff, 0x5000);
}

void uuid::generate(uuid_version ver, const gchar* name, int len)
{
    assert(name);
    switch(ver)
    {
    case uuid_v3:
        return generate_uuid_v3(this, name, len);
    case uuid_v5:
        return generate_uuid_v5(this, name, len);
    default:
        assert(!"unexpected uuid version.");
        return;
    }
}

void uuid::from_string(const gchar* str, int len)
{
    assert(str);
    gchar ss1[11] = _t("0x"),
        ss2[7] = _t("0x"),
        ss3[7] = _t("0x"),
        ss4[7] = _t("0x"),
        ss5[7] = _t("0x"),
        ss6[11] = _t("0x");
    int r = strtool::sscanf(str, _t("%[^-]-%[^-]-%[^-]-%[^-]-%4s%s"),
        ss1 + 2, _countof(ss1) - 2,
        ss2 + 2, _countof(ss2) - 2,
        ss3 + 2, _countof(ss3) - 2,
        ss4 + 2, _countof(ss4) - 2,
        ss5 + 2, _countof(ss5) - 2,
        ss6 + 2, _countof(ss6) - 2
        );
    if(r != 6) {
        assert(!"uuid from string failed.");
        return;
    }
    data1 = strtool::to_uint(ss1, 16);
    data2 = (uint16)strtool::to_uint(ss2, 16);
    data3 = (uint16)strtool::to_uint(ss3, 16);
    uint16 t1 = (uint16)strtool::to_uint(ss4, 16);
    uint16 t2 = (uint16)strtool::to_uint(ss5, 16);
    uint t3 = strtool::to_uint(ss6, 16);
    write_uint16(data4, t1);
    write_uint16(data4 + 2, t2);
    write_uint(data4 + 4, t3);
}

const gchar* uuid::to_string(string& str) const
{
    uint16 a, b;
    uint c;
    read_uint16(&a, data4);
    read_uint16(&b, data4 + 2);
    read_uint(&c, data4 + 4);
    str.format(_t("%08x-%04hx-%04hx-%04hx-%04hx%08x"),
        data1,
        data2,
        data3,
        a, b, c
        );
    return str.c_str();
}

void uuid::get_timestamp(uuid_timestamp& ts) const
{
    int64 time, t2, t3;
    t2 = data2;
    t3 = data3;
    time = data1 + (t2 << 32) + ((t3 & 0x0fff) << 48);
    time -= 0x01b21dd213814000;
    ts.tm_fraction = (int32)(time % 10000000);
    time /= (10 * 1000000);
    tm t;
    _localtime64_s(&t, &time);
    ts.tm_hour = t.tm_hour;
    ts.tm_mday = t.tm_mday;
    ts.tm_min = t.tm_min;
    ts.tm_mon = t.tm_mon;
    ts.tm_sec = t.tm_sec;
    ts.tm_wday = t.tm_wday;
    ts.tm_yday = t.tm_yday;
    ts.tm_yday = t.tm_year;
}

__gslib_end__
