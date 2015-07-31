#IOFireWall

###What is IOFireWall
IOFireWall is simple demo of fire wall on mac based on IOKit.

###Complonent of IOFireWall
1. kernel extension: it is used for get socket information.
                     For now, it is only hook 80 and 8080 port.

2. userland mode: have ring buffer to handle the information, and it will store in sqlite db for further review.

###Show
![image](https://github.com/dongAxis/IOFireWall/raw/master/img/pro.png)
