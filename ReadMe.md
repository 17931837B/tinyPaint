# TinyPaint 🎨

---

### **Colors**

* **White (0):** Eraser 🧼 (Overwrites with the background color)
* **Black (1):** Standard drawing color
* **Red (R):** Red drawing color
* **Green (G):** Green drawing color
* **Blue (B):** Blue drawing color

---

### **Brush Size**

Select a size from **2 to 8** to directly set the brush size.

* Current size: **30px**
* Range: **5px - 120px**

---

### **Controls**

| Action        | Key(s) / Mouse                 |
| :------------ | :----------------------------- |
| **Draw** | Left mouse drag 🖱️             |
| **Undo** | `Ctrl + Z`                     |
| **Redo** | `Ctrl + Shift + Z` or `Ctrl + Y` |
| **Save PNG** | `S`                            |
| **Save XPM** | `X`                            |

---

## TinyPaint's Featur ✨

---

### **1. Aspect Ratio is Maintained While Resizing**

Regardless of the window size, you can **adjust the display size while maintaining the aspect ratio** of the images.

<div align="center">
	<img src="/tmp/aspect1.png" width="30%"> <img src="/tmp/aspect2.png" width="50%">
</div>

---

### **2. Supports minilibx XML File Extension**

Great news for minilibx users: you can save images in the XML format (`*.xml`) by pressing the **X key**.

<div align="center">
	<img src="/tmp/cub.png" width="70%">
</div>

---  

# Linux環境でのOpeGL導入
1. セットアップもろもろ  
```
sudo apt-get install cmake pkg-config
sudo apt-get install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev 
sudo apt-get install libglew-dev libglfw3-dev libglm-dev 
sudo apt-get install libao-dev libmpg123-dev
mkdir lib && cd lib
sudo git clone https://github.com/glfw/glfw.git
sudo chmod -R a+rwx glfw
cd glfw
cmake .
make
sudo make install
```
2. GLADの生成  
   リンク先で画像ように設定して下にスクロールして```GENERATE```でzipファイルを生成
[Download GLAD](https://glad.dav1d.de/)
![alt text](tmp/Glad.png)
3. 適切に配置  
   zipファイルを展開するとincludeとsrcの２つのフォルダがある。src内のglad.cは作業用のディレクトリへ移す。include内のKHRフォルダとgladフォルダは```/usr/include```内に配置する。
   ```sudo cp -R include/* /usr/include/```
4. 作業用ディレクトリでコンパイル  
   ```sudo g++ (fail_name) glad.c -ldl -lglfw```