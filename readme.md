# Json解析器
## 文件说明

- **leptjsonC**

这是将Json文件转为C语言格式的数据。主要参考了叶劲峰（Milo Yip）老师的[json-tutorial](https://github.com/miloyip/json-tutorial)项目进行少量改写和实现的。主要编程语言风格是C语言。

- **leptjsonCplusplus**

这是在前面的基础上，将Json的C语言解析器转换为Json的C++语言解析器。期间，会尽量运用一些C++的高级语言技巧

- **JSON11**

这是这个项目的最终目标。看懂和实现Json11,源项目来自Dropbox的[Json11](https://github.com/dropbox/json11)里面运用许多C++11的特性

## Json介绍

Json是由[道格拉斯·克罗克福特](https://en.wikipedia.org/wiki/Douglas_Crockford)(Douglas Crockford)设计出来的，是一种轻量级的数据交换语言，易于阅读和理解。Json是Javascript的一个子集，数据格式与语言无关。JSON的文件扩展名是`.json`

## 预期成果

- 获得一个可以成功解析JSON数据格式的解析器，包括C和C++两个版本。
- 掌握cmake工具的使用方法
- 熟悉测试驱动开发（TDD）的过程

## 项目进展

- [x] leptjsonC
	- [x] leptjsonC-CH01 null和boolean型解析
	- [x] leptjsonC-CH02 number型解析
	- [x] leptjsonC-CH03 string型解析1
	- [x] leptjsonC-CH04 string型解析2
	- [x] leptjsonC-CH05 array型解析
	- [x] leptjsonC-CH06 object型解析
	- [x] leptjsonC-CH07 JSON生成器
	- [x] 20170824 加入内存泄漏检查（MS CRT） 
- [X] leptjsonCplusplus
	- [x] leptjsonCplusplus version 1.0 20170928 OK
	将leptjsonC转化为C++形式，通过所有测试用例
- [X] JSON11  
	Json11源代码已看懂。  20170929  OK
	
- [ ] 自己继续造轮子方向
	- [ ] 尝试加入异常机制来修改错误解析的情况
	- [ ] 尽可能加入智能指针
	
## 相关资料

1. [JSON][1]介绍
2. [JSON标准官方文件][4]　ECMA-404
2. [json-tutorial][2]叶老师项目地址
3. [JSON11][3]项目地址

[1]: http://json.org/     "Introducing JSON"
[2]: https://github.com/miloyip/json-tutorial "叶老师项目地址"
[3]: https://github.com/dropbox/json11 "JSON11"
[4]: http://link.zhihu.com/?target=http%3A//www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf
