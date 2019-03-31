# STM32F103-Robot-Car
The robot car is my project for the course EIE3105-Integrated Project at the Hong Kong Polytechnic University. It is a DC-motor-driven two-wheel robot car with a mobile power bank as power source, which is designed for handling multiple missions including following specific tracks and locating objects through the WIFI communication.

<p align="center">
  <img src="https://github.com/aaronzguan/STM32F103-Robot-Car/blob/master/photos/car.JPG" height="300">
  <img src="https://github.com/aaronzguan/STM32F103-Robot-Car/blob/master/photos/car_body.JPG" height="300">
  <img src="https://github.com/aaronzguan/STM32F103-Robot-Car/blob/master/photos/car_front.JPG" height="300">
</p>

The robot can perform three different tasks:

* [__Track Follower__](./Demo2-Track%20Follower): With the use of Infra-red Photo Transistors and SPI communication, the robot car can follow black track automatically. [Video for Track Follower](https://drive.google.com/open?id=1cUiWE4TCFwyTd62rdcmkwdAOJFBBLITv)

* [__Hit three balls__](Demo3-Hit%20Balls): The robot car can hit ball automatically by locating the position of the balls and car through WIFI communication. [Video for Hit Balls](https://drive.google.com/open?id=1hNRZpc35jRpEoIIF480u5co5YAhyImfd)

<p align="center">
  <img src="https://github.com/aaronzguan/STM32F103-Robot-Car/blob/master/photos/hitball.png" height="250">
</p>

* [__Pass ball__](Demo4-Pass%20Ball): Two robot cars, placed in two different regions, can be able to pass a ball to each other. Theoretically, with good control of the speed and orientation of two robot cars, they can pass the ball forever until the battery dies. [Video for Pass Ball](https://drive.google.com/open?id=15UgMaZLL-s1u7Mc-ZEMFmdfZflYVXLHK)

<p align="center">
  <img src="https://github.com/aaronzguan/STM32F103-Robot-Car/blob/master/photos/passball.png" height="250">
</p>

Here is the [Final Report](Final_Report.pdf).
