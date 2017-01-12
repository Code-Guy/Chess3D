# Chess3D
A 3D LAN chess game implemented with opengl4.x

![game](https://github.com/Code-Guy/Chess3D/blob/master/snapshots/game.png?raw=true)
###Introduction
This is a simple one-to-one 3d LAN chess game. First, you should input server's IP and port, connect to the server; Then you can start a standard chess game. The rule is just intuitive: You take a move, then wait your rival to move his piece.

 - on client side, connect the server

```
---------Network Stuff--------
Bind IP: 172.30.164.1, Bind Port: 2033
Please input the server's IP and Port:
172.30.164.1 2029
Connect to server successfully!
```
 - on server side, accept client's (only one) access.

```
---------Network Stuff--------
Bind IP: 172.30.164.1, Bind Port: 2029
Please input the server's IP and Port:
Client from 172.30.164.1:2037
```
###Feature
 - Network programming based on fundamental win32 socket
 - Implement a complete chess's rule system which players can interact with
 - Pick algorithm based on mouse raycasting and octree

 ###Framework
 Qt + Ork(a lightweight opengl wrapper)
 ###More Examples
![game](https://github.com/Code-Guy/Chess3D/blob/master/snapshots/main.png?raw=true)

![game](https://github.com/Code-Guy/Chess3D/blob/master/snapshots/side.png?raw=true)

![game](https://github.com/Code-Guy/Chess3D/blob/master/snapshots/far.png?raw=true)
