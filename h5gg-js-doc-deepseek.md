# H5GG JavaScript 脚本引擎文档 (v7.5) - DeepSeek整理版

## 目录
- [数值类型](#数值类型)
- [核心搜索与内存操作](#核心搜索与内存操作)
- [进程与模块管理](#进程与模块管理)
- [插件与扩展功能](#插件与扩展功能)
- [悬浮窗口控制](#悬浮窗口控制)
- [使用示例](#使用示例)
- [注意事项](#注意事项)
- [补充示例](#补充示例)

---

## 数值类型

| 类型 | 描述 | 字节大小 |
|------|------|----------|
| **F32** | 单精度浮点数 | 4字节 |
| **F64** | 双精度浮点数 | 8字节 |
| **I8** | 8位有符号整数 | 1字节 |
| **I16** | 16位有符号整数 | 2字节 |
| **I32** | 32位有符号整数 | 4字节 |
| **I64** | 64位有符号整数 | 8字节 |
| **U8** | 8位无符号整数 | 1字节 |
| **U16** | 16位无符号整数 | 2字节 |
| **U32** | 32位无符号整数 | 4字节 |
| **U64** | 64位无符号整数 | 8字节 |

---

## 核心搜索与内存操作

### `h5gg.require(version)`
设定脚本需要的最低H5GG版本号。

**参数：**
- `version`: 字符串，最低版本号

**示例：**
```javascript
h5gg.require('7.5.0'); // 脚本需要H5GG 7.5.0或更高版本
```

### `h5gg.setFloatTolerance(tolerance)`
设置F32/F64浮点搜索的偏差范围。

**参数：**
- `tolerance`: 字符串，浮点偏差值

**示例：**
```javascript
h5gg.setFloatTolerance('0.1'); // 设置0.1的浮点偏差
```

### `h5gg.searchNumber(value, type, startAddr, endAddr)`
搜索或二次搜索精确数值。

**参数：**
- `value`: 字符串，要搜索的数值（支持范围格式如"50~100"）
- `type`: 字符串，数值类型（如"F32"、"I32"等）
- `startAddr`: 字符串，搜索起始地址
- `endAddr`: 字符串，搜索结束地址

**返回值：** 无

**示例：**
```javascript
// 首次搜索
h5gg.searchNumber('100', 'I32', '0x100000000', '0x200000000');

// 范围搜索
h5gg.searchNumber('50~100', 'F32', '0x100000000', '0x200000000');
```

### `h5gg.searchNearby(value, type, range)`
邻近（联合）搜索。

**参数：**
- `value`: 字符串，要搜索的数值
- `type`: 字符串，数值类型
- `range`: 字符串，邻近范围（十六进制）

**示例：**
```javascript
h5gg.searchNearby('200', 'F32', '0x100');
```

### `h5gg.getValue(address, type)`
读取指定地址的数值。

**参数：**
- `address`: 字符串，内存地址
- `type`: 字符串，数值类型

**返回值：** 字符串，该地址的数值

**示例：**
```javascript
var value = h5gg.getValue('0x12345678', 'I32');
console.log('读取到的值：' + value);
```

### `h5gg.setValue(address, value, type)`
设置指定地址的数值。

**参数：**
- `address`: 字符串，内存地址
- `value`: 字符串或数字，要设置的值
- `type`: 字符串，数值类型

**返回值：** 布尔值，成功或失败

**示例：**
```javascript
var success = h5gg.setValue('0x12345678', '999', 'I32');
if(success) console.log('修改成功');
```

### `h5gg.editAll(value, type)`
修改搜索结果中的全部数值。

**参数：**
- `value`: 字符串，要修改的值
- `type`: 字符串，数值类型

**返回值：** 数字，修改成功的个数

**示例：**
```javascript
var count = h5gg.editAll('999', 'I32');
console.log('修改了' + count + '个值');
```

### `h5gg.getResultsCount()`
获取搜索结果的总数量。

**返回值：** 数字，结果总数

**示例：**
```javascript
var count = h5gg.getResultsCount();
console.log('找到' + count + '个结果');
```

### `h5gg.getResults(count, skip)`
获取结果数组。

**参数：**
- `count`: 数字，获取个数
- `skip`: 数字，跳过个数（可选）

**返回值：** 数组，每个元素包含`address`、`value`、`type`属性

**示例：**
```javascript
// 获取前100个结果
var results = h5gg.getResults(100);

// 跳过前50个，获取接下来100个
var results = h5gg.getResults(100, 50);

// 遍历结果
for(var i = 0; i < results.length; i++) {
	console.log('地址：' + results[i].address + 
				'，值：' + results[i].value + 
				'，类型：' + results[i].type);
}
```

### `h5gg.clearResults()`
清除搜索结果。

**示例：**
```javascript
h5gg.clearResults(); // 清空结果，开始新的搜索
```

### `fuckbase(address, bytes)`
修改静态基址。

**参数：**
- `address`: 字符串或数字，基址地址
- `bytes`: 字符串，十六进制字节码

**示例：**
```javascript
fuckbase(0x12345678, "00F0271E0008201E");
```

---

## 进程与模块管理

### `h5gg.setTargetProc(pid)`（跨进程版专用）
设定当前目标进程。

**参数：**
- `pid`: 数字，进程ID

**返回值：** 布尔值，成功或失败

**示例：**
```javascript
var success = h5gg.setTargetProc(1234);
if(success) console.log('已切换到进程1234');
```

### `h5gg.getProcList(processName)`（跨进程版专用）
获取进程数组。

**参数：**
- `processName`: 字符串，进程名（可选，不传入则返回所有进程）

**返回值：** 数组，元素包含`pid`(进程号)和`name`(进程名)属性

**示例：**
```javascript
// 获取所有进程
var allProcesses = h5gg.getProcList();

// 搜索特定进程
var targetProcesses = h5gg.getProcList('GameApp');

// 查找并设置目标进程
var processes = h5gg.getProcList('TargetGame');
if(processes.length > 0) {
	h5gg.setTargetProc(processes[0].pid);
}
```

### `h5gg.getRangesList(moduleName)`
返回模块数组。

**参数：**
- `moduleName`: 字符串，模块文件名（0=APP主程序，不传入=所有模块）

**返回值：** 数组，元素包含`start`(基址)、`end`(结尾地址)、`name`(路径)属性

**示例：**
```javascript
// 获取所有模块
var allModules = h5gg.getRangesList();

// 获取主程序模块
var mainModule = h5gg.getRangesList('0');

// 获取特定模块
var gameModule = h5gg.getRangesList('GameModule.dylib');

// 计算基址偏移
var module = h5gg.getRangesList('GameModule')[0];
var baseAddress = Number(module.start);
var targetAddress = baseAddress + 0x123456;
```

---

## 插件与扩展功能

### `h5gg.loadPlugin(className, dylibPath)`
加载dylib插件。

**参数：**
- `className`: 字符串，OC类名
- `dylibPath`: 字符串，dylib路径（绝对路径或.app中的相对路径）

**返回值：** OC对象实例

**示例：**
```javascript
var plugin = h5gg.loadPlugin('GamePlugin', '/path/to/plugin.dylib');
// 现在可以直接调用插件中的方法
plugin.doSomething();
```

---

## 悬浮窗口控制

### `setButtonImage(image)`
设置悬浮按钮图标。

**参数：**
- `image`: 字符串，http开头网址图片或base64编码的DataURL图片

**示例：**
```javascript
// 使用网络图片
setButtonImage('http://example.com/icon.png');

// 使用Base64图片
setButtonImage('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAA...');
```

### `setButtonAction(callback)`
设置自定义的悬浮按钮图标点击动作。

**参数：**
- `callback`: 函数，点击时调用的JS函数

**示例：**
```javascript
setButtonAction(function() {
	console.log('按钮被点击了！');
	// 执行自定义操作
	h5gg.searchNumber('100', 'I32', '0x0', '0xFFFFFFFF');
});
```

### `setWindowRect(x, y, width, height)`
修改悬浮窗口在屏幕中的位置和尺寸。

**参数：**
- `x`: 数字，X坐标
- `y`: 数字，Y坐标
- `width`: 数字，宽度
- `height`: 数字，高度

**示例：**
```javascript
setWindowRect(100, 100, 400, 500); // 设置窗口位置和大小
```

### `setWindowDrag(x, y, width, height)`
设置悬浮窗口中可拖动悬浮窗的区域。

**参数：**
- `x`: 数字，可拖动区域X坐标
- `y`: 数字，可拖动区域Y坐标
- `width`: 数字，可拖动区域宽度
- `height`: 数字，可拖动区域高度

**示例：**
```javascript
// 设置窗口顶部50像素为可拖动区域
setWindowDrag(0, 0, 370, 50);
```

### `setWindowTouch(touchable)`
设置悬浮窗口触摸穿透。

**参数：**
- `touchable`: 布尔值，true=整个悬浮窗口触摸不可穿透，false=可穿透

**示例：**
```javascript
setWindowTouch(true); // 窗口可触摸，不穿透
setWindowTouch(false); // 窗口不可触摸，穿透到底层应用
```

### `setWindowVisible(visible)`
设置悬浮窗口的可见性。

**参数：**
- `visible`: 布尔值，true=显示，false=隐藏

**示例：**
```javascript
setWindowVisible(true); // 显示窗口
setWindowVisible(false); // 隐藏窗口
```

### `setLayoutAction(callback)`
设置屏幕旋转或iPad分屏浮动变化时的回调函数。

**参数：**
- `callback`: 函数，回调函数，参数为(宽,高)

**示例：**
```javascript
setLayoutAction(function(width, height) {
	console.log('屏幕尺寸变化：' + width + 'x' + height);
	// 根据新尺寸调整UI
	if(width > height) {
		// 横屏模式
		setWindowRect(50, 50, 500, 300);
	} else {
		// 竖屏模式
		setWindowRect(50, 100, 370, 500);
	}
});
```

---

## 使用示例

### 基础搜索与修改
```javascript
// 搜索并修改F32类型值为10086的数据
h5gg.searchNumber('10086', 'F32', '0x100000000', '0x1600000000');
h5gg.editAll('10010', 'F32');
h5gg.clearResults();
```

### 联合搜索精确数值
```javascript
// 首次搜索
h5gg.searchNumber('10086', 'F32', '0x100000000', '0x1600000000');
// 邻近搜索，在附近100个字节内搜索1008611
h5gg.searchNearby('1008611', 'F32', '0x100');
// 二次搜索进一步筛选
h5gg.searchNumber('10086', 'F32');
// 修改所有结果
h5gg.editAll('10010', 'F32');
h5gg.clearResults();
```

### 修改指定内存地址与偏移
```javascript
h5gg.searchNumber('1078355558', 'I32', '0x100000000', '0x1600000000');
var count = h5gg.getResultsCount();
var results = h5gg.getResults(count);

for(var i = 0; i < count; i++) {
	var addr = results[i].address;
	
	// 检查地址是否以8D4结尾（注意：H5GG获取的地址是反写的）
	if(addr.endsWith('8D4')) {
		// 转换为数字进行偏移计算
		var addrNum = Number(addr);
		var offsetAddr = addrNum - 48; // 向上偏移12个四字节（48字节）
		
		// 修改原地址和偏移地址的值
		h5gg.setValue(addr, 18, "I32");
		h5gg.setValue('0x' + offsetAddr.toString(16), 0, "I32");
	}
}
h5gg.clearResults();
```

### 基址字节修改
```javascript
// 获取模块基址
var modules = h5gg.getRangesList('ShadowTrackerExtra');
if(modules.length > 0) {
	var baseAddr = Number(modules[0].start);
	var targetAddr = baseAddr + 0x02EF7144;
	
	// 修改基址字节
	fuckbase(targetAddr, "00F0271E0008201EC0035FD6");
}
```

### 循环修改
```javascript
var targetAddress = 0x1A2B3CD;

// 每10毫秒修改一次
setInterval(function() {
	h5gg.setValue(targetAddress, 999, "I32");
}, 10);
```

---

## 注意事项

1. **地址格式**：地址参数支持十进制或0x开头十六进制格式自动识别
2. **字符串转换**：搜索结果中的地址和数值都是字符串类型，做运算前需用`Number()`转换
3. **内存安全**：搜索结果较多时，不要一次性获取全部数据，应分段获取
4. **数组索引**：JavaScript数组索引从0开始（Lua从1开始）
5. **字符串连接**：JavaScript中`+`号在字符串间是连接操作，不是加法
6. **类型转换**：使用`toString(16)`将数字转为十六进制字符串前，确保变量是数字类型
7. **范围搜索**：支持"50~100"、"2.3~7.8"等范围格式
8. **窗口尺寸**：悬浮窗默认尺寸370x370，可通过接口调整

---

## 补充示例

### 分段获取搜索结果
```javascript
h5gg.searchNumber('999', 'I32', '0x0', '0xFFFFFFFF');
var total = h5gg.getResultsCount();
console.log('总共找到' + total + '个结果');

// 分段获取，每批1000个
var batchSize = 1000;
for(var i = 0; i < total; i += batchSize) {
	var results = h5gg.getResults(Math.min(batchSize, total - i), i);
	
	// 处理这批结果
	for(var j = 0; j < results.length; j++) {
		// 处理每个结果...
	}
	
	console.log('已处理' + Math.min(i + batchSize, total) + '/' + total);
}
h5gg.clearResults();
```

### 搜索并过滤特定模式
```javascript
// 搜索金币数值
h5gg.searchNumber('1000~999999', 'I32', '0x100000000', '0x200000000');
var results = h5gg.getResults(h5gg.getResultsCount());

var goldAddresses = [];
for(var i = 0; i < results.length; i++) {
	var value = Number(results[i].value);
	// 过滤：金币通常是100的倍数
	if(value % 100 === 0 && value > 0) {
		goldAddresses.push(results[i].address);
	}
}

console.log('找到' + goldAddresses.length + '个可能的金币地址');

// 修改所有金币地址
for(var i = 0; i < goldAddresses.length; i++) {
	h5gg.setValue(goldAddresses[i], '999999', 'I32');
}

h5gg.clearResults();
```

### 高级联合搜索模式
```javascript
// 搜索玩家生命值和最大生命值（通常相邻）
h5gg.searchNumber('100', 'F32'); // 搜索当前生命值
h5gg.searchNearby('150', 'F32', '0x10'); // 在附近搜索最大生命值

var results = h5gg.getResults(h5gg.getResultsCount());
console.log('找到' + results.length + '个生命值相关地址');

// 分组处理相邻地址
for(var i = 0; i < results.length; i += 2) {
	if(i + 1 < results.length) {
		var currentHP = results[i];
		var maxHP = results[i + 1];
		
		// 修改为无敌状态
		h5gg.setValue(currentHP.address, maxHP.value, 'F32');
	}
}

h5gg.clearResults();
```

### 创建简单GUI界面
```javascript
// 创建简单UI元素
function createSimpleUI() {
	// 创建标题
	var title = document.createElement('h3');
	title.textContent = 'H5GG脚本控制面板';
	title.style.textAlign = 'center';
	title.style.color = '#4CAF50';
	document.body.appendChild(title);
	
	// 创建按钮
	var button1 = document.createElement('button');
	button1.textContent = '搜索金币';
	button1.style.width = '100%';
	button1.style.padding = '10px';
	button1.style.margin = '5px 0';
	button1.onclick = function() {
		h5gg.searchNumber('1000~999999', 'I32');
		alert('搜索完成，找到' + h5gg.getResultsCount() + '个结果');
	};
	document.body.appendChild(button1);
	
	// 创建数值输入
	var input = document.createElement('input');
	input.type = 'number';
	input.placeholder = '输入要修改的值';
	input.style.width = '100%';
	input.style.padding = '8px';
	input.style.margin = '5px 0';
	document.body.appendChild(input);
	
	// 创建修改按钮
	var button2 = document.createElement('button');
	button2.textContent = '修改所有结果';
	button2.style.width = '100%';
	button2.style.padding = '10px';
	button2.style.margin = '5px 0';
	button2.onclick = function() {
		var value = input.value;
		if(value) {
			var count = h5gg.editAll(value, 'I32');
			alert('成功修改' + count + '个值');
		}
	};
	document.body.appendChild(button2);
}

// 页面加载完成后创建UI
if(document.readyState === 'loading') {
	document.addEventListener('DOMContentLoaded', createSimpleUI);
} else {
	createSimpleUI();
}
```

### 进程监控与自动切换
```javascript
// 自动监控目标进程是否运行
var targetPid = null;
var checkInterval = null;

function startProcessMonitor(processName) {
	checkInterval = setInterval(function() {
		var processes = h5gg.getProcList(processName);
		
		if(processes.length > 0) {
			var pid = processes[0].pid;
			if(targetPid !== pid) {
				// 进程已启动或切换
				h5gg.setTargetProc(pid);
				targetPid = pid;
				console.log('已切换到进程: ' + pid);
				onTargetProcessFound();
			}
		} else {
			// 进程未运行
			if(targetPid !== null) {
				console.log('目标进程已结束');
				targetPid = null;
				onTargetProcessLost();
			}
		}
	}, 2000); // 每2秒检查一次
}

function stopProcessMonitor() {
	if(checkInterval) {
		clearInterval(checkInterval);
		checkInterval = null;
	}
}

function onTargetProcessFound() {
	console.log('目标进程已找到，开始执行脚本...');
	// 这里可以放置针对目标进程的操作
}

function onTargetProcessLost() {
	console.log('目标进程丢失，停止操作...');
	// 清理操作
}

// 开始监控名为"GameApp"的进程
startProcessMonitor('GameApp');
```

这份文档涵盖了H5GG JavaScript脚本引擎的所有主要功能，包含详细的参数说明、返回值、使用示例和注意事项。你可以根据这些示例创建自己的游戏修改脚本。
