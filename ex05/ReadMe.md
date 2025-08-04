
#### fseek
ファイルポインタの操作に用いる  
```
int fseek(FILE *stream, long offset, int whence);
```
- stream
	操作したいファイルポインタ  
- offset
	基準点から何バイト移動させるか
- whence
  基準点の設定
