// Usage:
// #
// @!								-> begin
// @moveTo %f, %f;					-> x,y
// @lineTo %f, %f;					-> x,y
// @quadTo %f, %f, %f, %f;			-> cx1,cy1,x,y
// @cubicTo %f, %f, %f, %f, %f, %f;	-> cx1,cy1,cx2,cy2,x,y
// @lineExpression %f, %f, %f;		-> x,y,z
// @rect %f, %f, %f, %f;			-> left,top,right,bottom
// @dot %f, %f;						-> x,y
// @circle %f, %f, %f;				-> x,y,r
// @cross %f, %f;					-> x,y
// @tip %f, %f, %s;					-> x,y,text
// @@								-> done
// @&property=value;				-> modify properties
//

function DebugPath(obj) {
	// config
	this.config = {
		withArrow:true,
		withControlBoundary:true,
		strokeColor:"rgb(255,51,0)",
		strokeWidth:1.5,
		fillColor:"rgb(91,155,213)",
		bgColor:"rgb(255,255,255)",
		pkColor:"rgb(255,0,255)",
		width:0,
		height:0,
		scaleView:false,
		strokeDash:0
	};
	this.config.width = obj.width;
	this.config.height = obj.height;
	// context
	this.context = obj.getContext("2d");
	// path
	this.callings = new Array();
}

function clone(obj) {
	var o;
	switch(typeof obj) {
	case "undefined":
		break;
	case "string":
		o = obj + "";
		break;
	case "number":
		o = obj - 0;
		break;
	case "boolean":
		o = obj;
		break;
	case "object":
		if(obj == null)
			o = null;
		else {
			if(obj instanceof Array) {
				o = [];
				for(var i = 0, len = obj.length; i < len; i ++)
					o.push(clone(obj[i]));
			}
			else {
				o = {};
				for(var k in obj)
					o[k] = clone(obj[k]);
			}
		}
		break;
	default:
		o = obj;
		break;
	}
	return o;
}

DebugPath.prototype.config = null;
DebugPath.prototype.context = null;
DebugPath.prototype.callings = null;

DebugPath.prototype.begin = function() {
	var obj = { type: "begin" };
	this.callings.push(obj);
}

DebugPath.prototype.moveTo = function(x, y) {
	var obj = { type: "moveTo", data: [x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.lineTo = function(x, y) {
	var obj = { type: "lineTo", data: [x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.quadTo = function(cx1, cy1, x, y) {
	var obj = { type: "quadTo", data: [cx1, cy1, x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.cubicTo = function(cx1, cy1, cx2, cy2, x, y) {
	var obj = { type: "cubicTo", data: [cx1, cy1, cx2, cy2, x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.lineExpression = function(x, y, z) {
	var obj = { type: "lineExpression", data: [x, y, z] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.rect = function(left, top, right, bottom) {
	var obj = { type: "rect", data: [left, top, right, bottom] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.dot = function(x, y) {
	var obj = { type: "dot", data: [x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.circle = function(x, y, r) {
	var obj = { type: "circle", data: [x, y, r] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.cross = function(x, y) {
	var obj = { type: "cross", data: [x, y] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.tip = function(x, y, txt) {
	var obj = { type: "tip", data: [x, y, txt] };
	this.callings.push(clone(obj));
}

DebugPath.prototype.done = function() {
	var obj = { type: "done" };
	this.callings.push(obj);
}

DebugPath.prototype.modifyProperty = function(str) {
	var len = str.length;
	var end = str.indexOf(';');
	if(end == -1 || end > len)
		return str;
	var equ = str.substring(0, end);
	var eqPos = equ.indexOf('=');
	if(eqPos == -1)
		return;
	var lv = equ.substring(0, eqPos);
	var rv = equ.substring(eqPos + 1);
	if(lv.match("strokeColor"))
		this.config.strokeColor = rv;
	else if(lv.match("fillColor"))
		this.config.fillColor = rv;
	else if(lv.match("bgColor"))
		this.config.bgColor = rv;
	else if(lv.match("pkColor"))
		this.config.pkColor = rv;
	else if(lv.match("withArrow"))
		this.config.withArrow = rv;
	else if(lv.match("withControlBoundary"))
		this.config.withControlBoundary = rv;
	else if(lv.match("strokeWidth"))
		this.config.strokeWidth = rv;
	else if(lv.match("scaleView"))
		this.config.scaleView = rv;
	else if(lv.match("strokeDash"))
		this.config.strokeDash = rv;
	else
		alert("modifyProperty failed, bad l-value: " + lv);
	return str + end;
}

DebugPath.prototype.run = function() {
	var that = this;
	function runFunc(func) {
		var d = func.data;
		if(func.type == "moveTo")
			that.context.moveTo(d[0], d[1]);
		else if(func.type == "lineTo")
			that.context.lineTo(d[0], d[1]);
		else if(func.type == "quadTo")
			that.context.quadraticCurveTo(d[0], d[1], d[2], d[3]);
		else if(func.type == "cubicTo")
			that.context.bezierCurveTo(d[0], d[1], d[2], d[3], d[4], d[5]);
		else if(func.type == "rect") {
			that.context.moveTo(d[0], d[1]);
			that.context.lineTo(d[2], d[1]);
			that.context.moveTo(d[2], d[1]);
			that.context.lineTo(d[2], d[3]);
			that.context.moveTo(d[2], d[3]);
			that.context.lineTo(d[0], d[3]);
			that.context.moveTo(d[0], d[3]);
			that.context.lineTo(d[0], d[1]);
		}
		else if(func.type == "begin") {
			that.context.save();
			that.context.beginPath();
		}
		else if(func.type == "done")
			that.done1();
	}
	if(this.config.scaleView)
		this.scaling();
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		runFunc(node);
	}
}

DebugPath.prototype.prepare = function(str) {
	function getNextComment(str) {
		var s1 = str.indexOf('#');
		var s2 = str.indexOf('@');
		if(s1 == -1 || s1 > s2)
			return -1;
		return s1 + 1;
	}
	function skipCommentOnce(str) {
		var cs = getNextComment(str);
		if(cs == -1)
			return 0;
		var end = str.indexOf('\n', cs);
		if(end == -1)
			return str.length;
		return end;
	}
	function skipComment(str) {
		var sco = skipCommentOnce(str);
		if(sco == 0)
			return str;
		return skipComment(str.substring(sco));
	}
	function getCommandHeader(str) {
		var s = str.indexOf('@');
		if(s == -1)
			return 0;
		return str.substring(s + 1);
	}
	function fixedEqual(str, cmp) {
		var len = cmp.length;
		return str.substr(0, len).toLowerCase() == cmp.toLowerCase();
	}
	function walkCount(str, cmp) {
		var len = cmp.length + 1;
		var eos = str.indexOf(',', len);
		var eon = str.indexOf(';', len);
		if(eos == -1)
			return eon;
		else if(eon == -1)
			return eos;
		return eos < eon ? eos : eon;
	}
	function walkOver(str, cmp) {
		var step = walkCount(str, cmp);
		return str.substring(step);
	}
	function extractToken(str, cmp) {
		var end = walkCount(str, cmp);
		if(end == -1)
			return null;
		var tok = str.substring(cmp.length, end);
		tok.replace(/[^\d]/g, '');
		return tok;
	}
	while(str) {
		str = skipComment(str);
		if(!str)
			return;
		str = getCommandHeader(str);
		if(!str)
			return;
		if(fixedEqual(str, "!")) {
			this.begin();
		}
		else if(fixedEqual(str, "moveTo")) {
			var x = extractToken(str, "moveTo"); str = walkOver(str, "moveTo");
			var y = extractToken(str, ",");
			this.moveTo(x, y);
		}
		else if(fixedEqual(str, "lineTo")) {
			var x = extractToken(str, "lineTo"); str = walkOver(str, "lineTo");
			var y = extractToken(str, ",");
			this.lineTo(x, y);
		}
		else if(fixedEqual(str, "quadTo")) {
			var cx1 = extractToken(str, "quadTo"); str = walkOver(str, "quadTo");
			var cy1 = extractToken(str, ","); str = walkOver(str, ",");
			var x = extractToken(str, ","); str = walkOver(str, ",");
			var y = extractToken(str, ",");
			this.quadTo(cx1, cy1, x, y);
		}
		else if(fixedEqual(str, "cubicTo")) {
			var cx1 = extractToken(str, "cubicTo"); str = walkOver(str, "cubicTo");
			var cy1 = extractToken(str, ","); str = walkOver(str, ",");
			var cx2 = extractToken(str, ","); str = walkOver(str, ",");
			var cy2 = extractToken(str, ","); str = walkOver(str, ",");
			var x = extractToken(str, ","); str = walkOver(str, ",");
			var y = extractToken(str, ",");
			this.cubicTo(cx1, cy1, cx2, cy2, x, y);
		}
		else if(fixedEqual(str, "lineExpression")) {
			var x = extractToken(str, "lineExpression"); str = walkOver(str, "lineExpression");
			var y = extractToken(str, ","); str = walkOver(str, ",");
			var z = extractToken(str, ",");
			this.lineExpression(x, y, z);
		}
		else if(fixedEqual(str, "rect")) {
			var left = extractToken(str, "rect"); str = walkOver(str, "rect");
			var top = extractToken(str, ","); str = walkOver(str, ",");
			var right = extractToken(str, ","); str = walkOver(str, ",");
			var bottom = extractToken(str, ",");
			this.rect(left, top, right, bottom);
		}
		else if(fixedEqual(str, "dot")) {
			var x = extractToken(str, "dot"); str = walkOver(str, "dot");
			var y = extractToken(str, ",");
			this.dot(x, y);
		}
		else if(fixedEqual(str, "circle")) {
			var x = extractToken(str, "circle"); str = walkOver(str, "circle");
			var y = extractToken(str, ","); str = walkOver(str, ",");
			var r = extractToken(str, ",");
			this.circle(x, y, r);
		}
		else if(fixedEqual(str, "cross")) {
			var x = extractToken(str, "cross"); str = walkOver(str, "cross");
			var y = extractToken(str, ",");
			this.cross(x, y);
		}
		else if(fixedEqual(str, "tip")) {
			var x = extractToken(str, "tip"); str = walkOver(str, "tip");
			var y = extractToken(str, ","); str = walkOver(str, ",");
			var txt = extractToken(str, ";");
			this.tip(x, y, txt);
		}
		else if(fixedEqual(str, "@")) {
			this.done();
			str = str.substring(1);
		}
		else if(fixedEqual(str, "&")) {
			str = str.substring(1);
			str = this.modifyProperty(str);
		}
		else {
			alert("unexpected.");
		}
	}
}

DebugPath.prototype.scaling = function() {
	var left = 1.79e308,
		right = -1.79e308,
		top = 1.79e308,
		bottom = -1.79e308;
	var cap = this.callings.length;
	for(var i = 0; i < cap; i ++) {
		var item = this.callings[i];
		if(item.data && item.data.length > 0) {
			var len = item.data.length;
			if(len % 2 != 0)
				len --;
			for(var j = 0; j < len; j += 2) {
				var x = item.data[j];
				var y = item.data[j + 1];
				left = Math.min(x, left);
				top = Math.min(y, top);
				right = Math.max(x, right);
				bottom = Math.max(y, bottom);
			}
		}
	}
	var width = right - left;
	var height = bottom - top;
	var sx = (this.config.width - 10) / width;
	var sy = (this.config.height - 10) / height;
	var s = Math.min(sx, sy);
	// centralize
	var width1 = width * s;
	var height1 = height * s;
	var cx = (this.config.width - width1) / 2 + 5;
	var cy = (this.config.height - height1) / 2 + 5;
	cx -= left * s;
	cy -= top * s;
	for(var i = 0; i < cap; i ++) {
		var item = this.callings[i];
		if(item.data && item.data.length > 0) {
			var len = item.data.length;
			for(var j = 0; j < len; j += 2) {
				item.data[j] = item.data[j] * s + cx;
				item.data[j + 1] = item.data[j + 1] * s + cy;
			}
		}
	}
}

DebugPath.prototype.done1 = function() {
	if(this.config.fillColor != null) {
		this.context.fillStyle = this.config.fillColor;
		this.context.fill();
	}
	if(this.config.strokeColor != null) {
		this.context.strokeStyle = this.config.strokeColor;
		this.context.lineWidth = this.config.strokeWidth;
		if(this.config.strokeDash != 0)
			this.context.setLineDash([5]);
		this.context.stroke();
	}
	if(this.config.withControlBoundary)
		this.drawControlBoundary();
	if(this.config.withArrow)
		this.drawArrow();
	this.drawLineExpressions();
	this.drawDots();
	this.drawCircles();
	this.drawCrosses();
	this.drawTips();
	this.context.restore();
}

DebugPath.prototype.drawControlBoundary = function() {
	function getDestPoint(node) {
		var arr = node.data;
		var len = arr.length;
		return [arr[len - 2], arr[len - 1]];
	}
	this.context.save();
	this.context.strokeStyle = "rgb(0,255,0)";
	this.context.lineWidth = 1.5;
	this.context.setLineDash([5]);
	this.context.beginPath();
	for(var i = 0, len = this.callings.length; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "quadTo") {
			var lastPos = getDestPoint(this.callings[i - 1]);
			this.context.moveTo(lastPos[0], lastPos[1]);
			this.context.lineTo(node.data[0], node.data[1]);
			this.context.lineTo(node.data[2], node.data[3]);
		}
		else if(node.type == "cubicTo") {
			var lastPos = getDestPoint(this.callings[i - 1]);
			this.context.moveTo(lastPos[0], lastPos[1]);
			this.context.lineTo(node.data[0], node.data[1]);
			this.context.lineTo(node.data[2], node.data[3]);
			this.context.lineTo(node.data[4], node.data[5]);
		}
	}
	this.context.stroke();
	this.context.restore();
}

DebugPath.prototype.drawArrowShape = function(lastPos) {
	var start, dir, pos;
	var lastNode = this.callings[lastPos];
	if(lastNode.type == "moveTo" || lastNode.type == "lineTo") {
		var prevNode = this.callings[lastPos - 1];
		var arr1 = lastNode.data;
		var arr2 = prevNode.data;
		var len = arr2.length;
		start = [arr2[len - 2], arr2[len - 1]];
		pos = [arr1[0], arr1[1]];
	}
	else {
		var arr = lastNode.data;
		var len = arr.length;
		start = [arr[len - 4], arr[len - 3]];
		pos = [arr[len - 2], arr[len - 1]];
	}
	dir = [pos[0] - start[0], pos[1] - start[1]];
	var arrowLen = 8;
	this.context.save();
	this.context.fillStyle = this.config.strokeColor;
	this.context.beginPath();
	this.context.translate(pos[0], pos[1]);
	this.context.rotate(Math.atan2(dir[1], dir[0]) + Math.PI * 0.5);
	this.context.moveTo(0, 0);
	this.context.lineTo(arrowLen / 2, arrowLen);
	this.context.lineTo(0, arrowLen * 0.6);
	this.context.lineTo(-arrowLen / 2, arrowLen);
	this.context.lineTo(0, 0);
	this.context.fill();
	this.context.restore();
}

DebugPath.prototype.drawArrow = function() {
	function lastPathNode(callings, p) {
		for(var i = p; i >= 0; i --) {
			var node = callings[i];
			if(node.type == "moveTo" ||
				node.type == "lineTo" ||
				node.type == "quadTo" ||
				node.type == "cubicTo"
				)
				return i;
		}
		return -1;
	}
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		if(this.callings[i].type == "moveTo") {
			var lastPos = lastPathNode(this.callings, i - 1);
			if(lastPos > 0)
				this.drawArrowShape(lastPos);
		}
	}
	var lastPos = lastPathNode(this.callings, len - 1);
	if(lastPos > 0)
		this.drawArrowShape(lastPos);
}

DebugPath.prototype.drawDots = function() {
	this.context.save();
	this.context.fillStyle = "rgb(255,255,0)";
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "dot") {
			var x = node.data[0], y = node.data[1];
			const r = 2;
			this.context.beginPath();
			this.context.arc(x, y, r, 0, Math.PI * 2, true);
			this.context.closePath();
			this.context.fill();
		}
	}
	this.context.restore();
}

DebugPath.prototype.drawCircles = function() {
	this.context.save();
	this.context.strokeStyle = "rgb(237,125,49)";
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "circle") {
			var x = node.data[0], y = node.data[1], r = node.data[2];
			this.context.beginPath();
			this.context.arc(x, y, r, 0, Math.PI * 2, true);
			this.context.closePath();
			this.context.stroke();
		}
	}
	this.context.restore();
}

DebugPath.prototype.drawCrosses = function() {
	this.context.save();
	this.context.strokeStyle = "rgb(0,128,128)";
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "cross") {
			var x = parseFloat(node.data[0]), y = parseFloat(node.data[1]);
			this.context.beginPath();
			this.context.moveTo(x - 4, y - 4);
			this.context.lineTo(x + 4, y + 4);
			this.context.moveTo(x - 4, y + 4);
			this.context.lineTo(x + 4, y - 4);
			this.context.closePath();
			this.context.stroke();
		}
	}
	this.context.restore();
}

DebugPath.prototype.drawLineExpressions = function() {
	this.context.save();
	this.context.strokeStyle = "rgb(200,0,200)";
	this.context.setLineDash([2,5]);
	var getXByY = function(coef, y) {
		if(parseFloat(coef[0]) == 0)
			return -1;
		return -(parseFloat(coef[2]) + parseFloat(coef[1]) * y) / parseFloat(coef[0]);
	}
	var getYByX = function(coef, x) {
		if(parseFloat(coef[1]) == 0)
			return -1;
		return -(parseFloat(coef[2]) + parseFloat(coef[0]) * x) / parseFloat(coef[1]);
	}
	var len = this.callings.length;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "lineExpression") {
			var lineSample = new Array;
			var t = getYByX(node.data, 0);
			if(t >= 0 && t <= this.config.height) {
				lineSample.push(0);
				lineSample.push(t);
			}
			t = getYByX(node.data, this.config.width);
			if(t >= 0 && t <= this.config.height) {
				lineSample.push(this.config.width);
				lineSample.push(t);
			}
			t = getXByY(node.data, 0);
			if(t >= 0 && t <= this.config.width) {
				lineSample.push(t);
				lineSample.push(0);
			}
			t = getXByY(node.data, this.config.height);
			if(t >= 0 && t <= this.config.width) {
				lineSample.push(t);
				lineSample.push(this.config.height);
			}
			if(lineSample.length == 4) {
				this.context.beginPath();
				this.context.moveTo(lineSample[0], lineSample[1]);
				this.context.lineTo(lineSample[2], lineSample[3]);
				this.context.stroke();
			}
		}
	}
	this.context.restore();
}

DebugPath.prototype.drawTips = function() {
	this.context.save();
	this.context.fillStyle = "rgb(97,13,221)";
	var len = this.callings.length;
	const offsetX = 0;
	const offsetY = 5;
	for(var i = 0; i < len; i ++) {
		var node = this.callings[i];
		if(node.type == "tip") {
			this.context.fillText(node.data[2], parseFloat(node.data[0]) + offsetX, parseFloat(node.data[1]) + offsetY);
		}
	}
	this.context.restore();
}
