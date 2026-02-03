# H5GG JavaScript 脚本开发文档 (v7.5) - Kimi整理版

## 一、基础数据类型

H5GG支持以下数值类型，用于内存搜索和修改：

| 类型标识 | 说明 | 字节长度 | 取值范围 |
|---------|------|---------|---------|
| `F32` | 单精度浮点数 | 4字节 | ±3.4E±38 |
| `F64` | 双精度浮点数 | 8字节 | ±1.7E±308 |
| `I8` | 有符号8位整数 | 1字节 | -128 ~ 127 |
| `I16` | 有符号16位整数 | 2字节 | -32768 ~ 32767 |
| `I32` | 有符号32位整数 | 4字节 | -2147483648 ~ 2147483647 |
| `I64` | 有符号64位整数 | 8字节 | -9223372036854775808 ~ 9223372036854775807 |
| `U8` | 无符号8位整数 | 1字节 | 0 ~ 255 |
| `U16` | 无符号16位整数 | 2字节 | 0 ~ 65535 |
| `U32` | 无符号32位整数 | 4字节 | 0 ~ 4294967295 |
| `U64` | 无符号64位整数 | 8字节 | 0 ~ 18446744073709551615 |

---

## 二、核心API接口

### 2.1 引擎控制

#### `h5gg.require(version)`
设定脚本所需的最低H5GG版本号，建议写在脚本第一行。

```javascript
h5gg.require('7.5');  // 要求H5GG版本不低于7.5
```

#### `h5gg.setFloatTolerance(tolerance)`
设置浮点搜索的偏差范围，默认为0.0（精确匹配）。

```javascript
h5gg.setFloatTolerance('0.0001');  // 允许0.0001的误差
```

---

### 2.2 内存搜索

#### `h5gg.searchNumber(value, type, startAddr, endAddr)`
在指定内存范围搜索精确数值。

| 参数 | 类型 | 说明 |
|------|------|------|
| `value` | string | 搜索值，支持范围格式如 `"50~100"` 或 `"2.3~7.8"` |
| `type` | string | 数值类型，如 `F32`, `I32` 等 |
| `startAddr` | string | 起始地址（十进制或0x开头的十六进制） |
| `endAddr` | string | 结束地址（十进制或0x开头的十六进制） |

**返回值：** 无（结果存入内部缓冲区）

```javascript
// 搜索F32类型值为10086的数据
h5gg.searchNumber('10086', 'F32', '0x100000000', '0x1600000000');
```

#### `h5gg.searchNearby(value, type, range)`
邻近（联合）搜索，在上次搜索结果附近进行过滤。

| 参数 | 类型 | 说明 |
|------|------|------|
| `value` | string | 邻近搜索的目标值 |
| `type` | string | 数值类型 |
| `range` | string | 邻近范围（十六进制），如 `'0x100'` 表示 ±256字节 |

```javascript
// 在第一次搜索结果附近0x100字节范围内搜索1008611
h5gg.searchNumber('10086', 'F32', '0x100000000', '0x1600000000');
h5gg.searchNearby('1008611', 'F32', '0x100');
```

#### `h5gg.clearResults()`
清空搜索结果，释放内存。在开启新搜索前必须调用。

```javascript
h5gg.clearResults();
```

---

### 2.3 结果获取与修改

#### `h5gg.getResultsCount()`
获取当前搜索结果的总数量。

**返回值：** number（结果总数）

```javascript
var count = h5gg.getResultsCount();
alert('找到 ' + count + ' 个结果');
```

#### `h5gg.getResults(count, skip)`
获取搜索结果数组。

| 参数 | 类型 | 说明 |
|------|------|------|
| `count` | number | 获取个数（建议分段获取，避免内存溢出） |
| `skip` | number | 跳过个数（用于分页） |

**返回值：** Array\<Object\>，每个元素包含：
- `address`: string（内存地址，十六进制字符串）
- `value`: string（当前数值）
- `type`: string（数值类型）

⚠️ **警告**：结果较多时不要一次性获取全部，应分段获取避免闪退。

```javascript
// 分段获取，每次100条
var total = h5gg.getResultsCount();
for(var i = 0; i < total; i += 100) {
	var batch = h5gg.getResults(100, i);
	batch.forEach(function(item) {
		console.log('地址：' + item.address + ' 值：' + item.value);
	});
}
```

#### `h5gg.editAll(value, type)`
修改所有搜索结果中的数值。

| 参数 | 类型 | 说明 |
|------|------|------|
| `value` | string | 新数值 |
| `type` | string | 数值类型 |

**返回值：** number（成功修改的个数）

```javascript
// 将所有搜索结果修改为10010
var modified = h5gg.editAll('10010', 'F32');
alert('成功修改 ' + modified + ' 个地址');
```

#### `h5gg.setValue(address, value, type)`
修改指定地址的数值。

| 参数 | 类型 | 说明 |
|------|------|------|
| `address` | string | 目标地址（支持十进制或0x开头） |
| `value` | number/string | 新数值 |
| `type` | string | 数值类型 |

**返回值：** boolean（是否成功）

```javascript
h5gg.setValue('0x1A2B3CD', 999, 'I32');
```

#### `h5gg.getValue(address, type)`
读取指定地址的数值。

| 参数 | 类型 | 说明 |
|------|------|------|
| `address` | string | 目标地址 |
| `type` | string | 数值类型 |

**返回值：** string（数值字符串，需用 `Number()` 转换后计算）

```javascript
var val = h5gg.getValue('0x1A2B3CD', 'I32');
var num = Number(val) + 100;  // 转为数字进行运算
```

---

### 2.4 模块与基址操作

#### `h5gg.getRangesList(moduleName)`
获取模块内存范围列表。

| 参数 | 类型 | 说明 |
|------|------|------|
| `moduleName` | string | 模块文件名（如 `'ShadowTrackerExtra'`）<br>`'0'` - 返回APP主程序模块<br>不传参 - 返回所有模块列表 |

**返回值：** Array\<Object\>，每个元素包含：
- `start`: string（模块基址）
- `end`: string（模块结束地址）
- `name`: string（模块路径）

```javascript
// 获取主程序模块信息
var mainModule = h5gg.getRangesList('0');
var baseAddr = mainModule[0].start;

// 获取特定游戏模块
var game = h5gg.getRangesList('ShadowTrackerExtra');
var gameBase = game[0].start;
```

#### `fuckbase(address, hexBytes)`
修改静态基址的字节数据（通常用于Patch指令）。

| 参数 | 类型 | 说明 |
|------|------|------|
| `address` | number | 目标地址（需为数字类型） |
| `hexBytes` | string | 十六进制字节串（无空格，如 `"00F0271E"`） |

```javascript
var module = h5gg.getRangesList('ShadowTrackerExtra');
var addr = Number(module[0].start) + 0x02EF7144;
fuckbase(addr, "00F0271E0008201EC0035FD6");
```

---

### 2.5 跨进程操作（跨进程版专用）

#### `h5gg.getProcList(procName)`
获取进程列表。

| 参数 | 类型 | 说明 |
|------|------|------|
| `procName` | string | 进程名（可选，不传返回所有APP进程） |

**返回值：** Array\<Object\>，每个元素包含：
- `pid`: number（进程号）
- `name`: string（进程名）

```javascript
// 查找特定游戏进程
var procs = h5gg.getProcList('ShadowTrackerExtra');
if(procs.length > 0) {
	var pid = procs[0].pid;
	h5gg.setTargetProc(pid);
}
```

#### `h5gg.setTargetProc(pid)`
设定当前操作的目标进程。

| 参数 | 类型 | 说明 |
|------|------|------|
| `pid` | number | 进程号 |

**返回值：** boolean（是否成功）

```javascript
// 定时检查进程是否存在
setInterval(function() {
	var list = h5gg.getProcList('目标APP');
	if(list.length === 0) {
		alert('目标进程已结束');
	}
}, 3000);
```

---

### 2.6 插件系统

#### `h5gg.loadPlugin(className, dylibPath)`
加载dylib插件并返回OC类实例。

| 参数 | 类型 | 说明 |
|------|------|------|
| `className` | string | OC类名 |
| `dylibPath` | string | dylib路径（支持绝对路径或.app内相对路径） |

**返回值：** Object（OC对象实例，可在JS中直接调用其方法）

```javascript
var plugin = h5gg.loadPlugin('MyPlugin', '/path/to/plugin.dylib');
plugin.someMethod();  // 调用OC方法
```

---

### 2.7 UI悬浮窗控制

#### `setButtonImage(image)`
设置悬浮按钮图标。

| 参数 | 类型 | 说明 |
|------|------|------|
| `image` | string | HTTP图片URL 或 Base64编码的DataURL |

```javascript
setButtonImage('https://example.com/icon.png');
// 或
setButtonImage('data:image/png;base64,iVBORw0KGgoAAAANSUhEUg...');
```

#### `setButtonAction(callback)`
设置悬浮按钮点击回调。

```javascript
setButtonAction(function() {
	alert('按钮被点击了');
});
```

#### `setWindowRect(x, y, width, height)`
设置悬浮窗口位置和尺寸（默认370x370）。

```javascript
setWindowRect(100, 100, 400, 600);
```

#### `setWindowDrag(x, y, width, height)`
设置可拖动区域。

```javascript
// 顶部40像素高度为拖动区域
setWindowDrag(0, 0, 370, 40);
```

#### `setWindowTouch(enable)`
设置触摸穿透。

| 参数 | 类型 | 说明 |
|------|------|------|
| `enable` | boolean | `true` - 窗口响应触摸（不可穿透）<br>`false` - 窗口不响应触摸（可穿透） |

```javascript
setWindowTouch(false);  // 触摸穿透，不影响游戏操作
```

#### `setWindowVisible(visible)`
设置窗口可见性。

```javascript
setWindowVisible(false);  // 隐藏窗口
setWindowVisible(true);   // 显示窗口
```

#### `setLayoutAction(callback)`
屏幕旋转或分屏变化时的回调。

```javascript
setLayoutAction(function(width, height) {
	console.log('窗口尺寸变化：' + width + 'x' + height);
	// 可在此重新调整UI布局
});
```

---

## 三、完整示例

### 示例1：基础搜索与修改
```javascript
h5gg.require('7.5');

// 搜索金币数值
h5gg.searchNumber('9999', 'I32', '0x100000000', '0x1600000000');

// 如果结果太多，进行二次搜索（联合搜索）
if(h5gg.getResultsCount() > 100) {
	h5gg.searchNearby('100', 'F32', '0x200');
}

// 修改所有结果为999999
var count = h5gg.editAll('999999', 'I32');
alert('修改完成，共 ' + count + ' 处');

h5gg.clearResults();
```

### 示例2：基址偏移修改（以4D8结尾的地址）
```javascript
h5gg.searchNumber('1078355558', 'I32', '0x100000000', '0x1600000000');
var count = h5gg.getResultsCount();
var results = h5gg.getResults(count);

for(var i = 0; i < count; i++) {
	var addr = results[i].address;
	
	// 正则匹配以8D4结尾的地址（注意：H5GG地址是反写的）
	if(/8D4$/.test(addr)) {
		var base = Number(addr);
		var offset1 = base - 48;  // 向上偏移12个I32（12*4=48）
		var offset2 = base;
		
		h5gg.setValue('0x' + offset1.toString(16), 0, 'I32');
		h5gg.setValue('0x' + offset2.toString(16), 18, 'I32');
	}
}
h5gg.clearResults();
```

### 示例3：循环锁定数值（防闪退）
```javascript
var targetAddr = '0x1A2B3CD';

// 使用setInterval而非while循环，避免单线程阻塞
var locker = setInterval(function() {
	h5gg.setValue(targetAddr, 9999, 'I32');
}, 100);  // 每100毫秒刷新一次

// 停止锁定：clearInterval(locker);
```

### 示例4：模块基址Patch
```javascript
// 获取游戏主模块
var ranges = h5gg.getRangesList('ShadowTrackerExtra');
if(ranges.length > 0) {
	var base = Number(ranges[0].start);
	var patchAddr = base + 0x02EF7144;
	
	// Patch为NOP指令 (ARM64: NOP = 0xD503201F)
	fuckbase(patchAddr, "1F2003D5");
	alert('基址修改成功');
}
```

---

## 四、高级示例：内存特征码扫描

原文档未提及，这是一个实用的**特征码扫描**示例，用于定位动态变化的内存地址：

```javascript
/**
 * 特征码扫描功能
 * 通过字节特征码定位内存地址，支持"??"通配符
 * @param {string} moduleName - 模块名
 * @param {string} pattern - 特征码，如 "12 34 ?? 56 78"
 * @returns {string|null} - 找到的地址或null
 */
function scanPattern(moduleName, pattern) {
	// 获取模块范围
	var ranges = h5gg.getRangesList(moduleName);
	if(ranges.length === 0) return null;
	
	var start = Number(ranges[0].start);
	var end = Number(ranges[0].end);
	var rangeSize = end - start;
	
	// 将特征码转为数组
	var bytes = pattern.split(' ').map(function(b) {
		return b === '??' ? -1 : parseInt(b, 16);
	});
	
	// 分段搜索，避免内存溢出（每次搜索1MB）
	var step = 0x100000;
	for(var addr = start; addr < end; addr += step) {
		var searchEnd = Math.min(addr + step, end);
		
		// 搜索第一个字节
		var firstByte = bytes.find(function(b) { return b !== -1; });
		h5gg.clearResults();
		h5gg.searchNumber(firstByte.toString(), 'U8', 
			'0x' + addr.toString(16), 
			'0x' + searchEnd.toString(16));
		
		var count = h5gg.getResultsCount();
		if(count === 0) continue;
		
		// 获取结果并验证完整特征码
		var results = h5gg.getResults(Math.min(count, 1000), 0);
		
		for(var i = 0; i < results.length; i++) {
			var baseAddr = Number(results[i].address);
			var match = true;
			
			// 验证后续字节
			for(var j = 0; j < bytes.length; j++) {
				if(bytes[j] === -1) continue; // 通配符跳过
				
				var checkAddr = '0x' + (baseAddr + j).toString(16);
				var val = Number(h5gg.getValue(checkAddr, 'U8'));
				
				if(val !== bytes[j]) {
					match = false;
					break;
				}
			}
			
			if(match) {
				h5gg.clearResults();
				return '0x' + baseAddr.toString(16);
			}
		}
		
		h5gg.clearResults();
	}
	
	return null;
}

// 使用示例：搜索 "12 34 ?? 56 78" 的特征码
var addr = scanPattern('ShadowTrackerExtra', '12 34 ?? 56 78');
if(addr) {
	alert('特征码找到于: ' + addr);
	// 在此地址基础上进行偏移计算
	var target = Number(addr) + 0x100;
	h5gg.setValue('0x' + target.toString(16), 999, 'I32');
} else {
	alert('未找到特征码');
}
```

---

## 五、重要注意事项

1. **地址格式**：地址参数支持十进制或 `0x` 开头的十六进制自动识别，**其他参数必须是字符串格式**。

2. **数值运算**：搜索结果中的 `address` 和 `value` 都是**字符串类型**，进行数学运算前必须用 `Number()` 转换。
   ```javascript
   var addr = Number(r[0].address) + 0x100;  // 正确
   // var addr = r[0].address + 0x100;	   // 错误：字符串拼接
   ```

3. **数组索引**：Lua数组从 `[1]` 开始，**JavaScript数组从 `[0]` 开始**。

4. **地址反写**：H5GG获取的内存地址是反写的（小端序）。如需匹配以 `4D8` 结尾的地址，代码中应判断 `8D4$`。

5. **地址偏移计算**：每个I32/F32偏移为4字节，I64/F64为8字节。
   ```javascript
   // 向上偏移3个I32 = 后退12字节（地址减小）
   var newAddr = Number(baseAddr) + (-12);  // 或 -0xC
   ```

6. **内存管理**：搜索结果较多时**切勿一次性 `getResults` 获取全部**，应分段获取（建议每次不超过1000条）。

7. **十六进制转换**：
   ```javascript
   var hex = (12345).toString(16);		// "3039"
   var num = parseInt("3039", 16);		// 12345
   ```

8. **范围搜索**：`searchNumber` 支持范围格式，如 `"50~100"` 或 `"2.3~7.8"`。

9. **清空结果**：每次搜索前必须调用 `h5gg.clearResults()`，否则会在上次结果中二次搜索。

10. **循环修改**：JS是单线程，使用 `while` 循环会导致卡顿甚至闪退，**必须使用 `setInterval`**。

---

以上文档涵盖了H5GG JS脚本开发的核心功能，建议根据实际需求组合使用 `searchNumber` + `searchNearby` 进行精确地址定位，再配合 `setInterval` 实现数值锁定。
