# 财务助手

[![github](https://img.shields.io/badge/github-Juncylee-brightgreen.svg)](https://github.com/Juncylee)


本项目读取根目录下 `data.json` 中的交易分析数据，并调用 [DeepSeek 大模型 API](https://api-docs.deepseek.com/zh-cn/)，生成理财建议。

### 须知
目前只有 Windows 版本。

### 环境准备

安装 MinGW-w64（[通过 MSYS2 安装](https://www.msys2.org/)）进行编译。

### 必备工具安装步骤

   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake curl
