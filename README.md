# Self-Driving Car Engineer Nanodegree

## Term 2 Project 3: Kidnapped Vehicle

The goal for this project was to implement a particle filter to localize a kidnapped vehicle. The project specification was given [here](https://review.udacity.com/#!/rubrics/747/view).

### Approach

There was a lot of theoretical information during the courses preparing for this project. In the end the course "Implementation of particle filter" was pretty useful for the implementation and led the way for the running code. So I stayed close to the prepared functions and the implementation in the code.

### Challenges

When I had finished the code and sucessfully compiled it, I started the simulator and instantly got an "Segmentation fault: 11". But searching the forum I could identify the missing line. I just did not initialize the weigths vector. After correction the code run well and I got the confirmation of success from the simulator.

### Conclusion

The implementation of this project was similar to the Kalman filters pretty fast forward using the recommendations from the course.
After initializing all the the variables the code was sucessful in the simulator.
![sucessful running code](/images/program_success.png)
