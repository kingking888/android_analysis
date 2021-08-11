
 ## 编译方法
 unix like
 ```shell
mkdir "build"
cd build
cmake .. -DNDK=your_ndk_path/Android/sdk/ndk/22.0.7026061 -DANDROID_ABI=armeabi-v7a
make -j8
```
或者使用andriod studio编译
## 注入方法
采用frida注入
```
把生成的libnative_hook*.so放入/data/app/目录下
```
```
在inject文件夹中
main.py修改要注入的进程
index.js中修改要注入的so
```
记得把.json文件放在sdcard

bug反馈群:852381204