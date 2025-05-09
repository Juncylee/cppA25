# 1. 提取数据
python data_extractor.py

# 2. 编译 C++
g++ -std=c++17 main.cpp -lcurl -lssl -lcrypto -o finance-helper.exe

# 3. 运行
finance-helper.exe