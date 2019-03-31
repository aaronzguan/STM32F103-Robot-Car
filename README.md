# STM32F103-Robot-Car
The robot car is my project for the course EIE3105-Integrated Project at the Hong Kong Polytechnic University. It is a DC-motor-driven two-wheel robot car with a mobile power bank as power source, which is designed for handling multiple missions including following specific tracks and locating objects through the WIFI communication.

![](/photos/car.JPG =50x20)
![](/photos/car_body.JPG =50x20)
![](/photos/car_front.JPG =50x20)

The robot can perform three different tasks:

* __Track Follower:__ With the use of Infra-red Photo Transistors and SPI communication, the robot car can follow black track automatically. [Video for Track Follower](https://drive.google.com/open?id=1cUiWE4TCFwyTd62rdcmkwdAOJFBBLITv)

* __Hit three balls:__ The robot car can hit ball automatically by locating the position of the balls and car through WIFI communication. [Video for Hit Balls](https://drive.google.com/open?id=1hNRZpc35jRpEoIIF480u5co5YAhyImfd)

![](/photos/hitball.png)

* __Pass ball:__ Two robot cars, placed in two different regions, can be able to pass a ball to each other. Theoretically, with good control of the speed and orientation of two robot cars, they can pass the ball forever until the battery dies. Video for Pass Ball](https://drive.google.com/open?id=15UgMaZLL-s1u7Mc-ZEMFmdfZflYVXLHK)

![](/photos/passball.png)

Here is the [Final Report](Final_Report.pdf).
