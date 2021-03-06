/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
    num_particles = 200;
    // generating noise
    default_random_engine gen;
    normal_distribution<double> N_x_init(x, std[0]);
    normal_distribution<double> N_y_init(y, std[1]);
    normal_distribution<double> N_theta_init(theta, std[2]);
    particles = vector<Particle>(num_particles);
    weights = std::vector<double>(num_particles);
    // initalize particles with gaussian noise
    for (int i = 0; i < num_particles; i++ ){
        particles[i].weight = 1.0;
        particles[i].x = N_x_init(gen);
        particles[i].y = N_y_init(gen);
        particles[i].theta = N_theta_init(gen);
        particles[i].id = i;
        weights[i] = 1.0;
    }
    is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
    // generating noise
    default_random_engine gen;
    normal_distribution<double> N_x(0, std_pos[0]);
    normal_distribution<double> N_y(0, std_pos[1]);
    normal_distribution<double> N_theta(0, std_pos[2]);
    // predicting x, y and theta with dependency to value of yaw_rate
    for (int i = 0; i < particles.size(); i++){
        double x = particles[i].x;
        double y = particles[i].y;
        double theta = particles[i].theta;
        if (fabs(yaw_rate) < 1e-6){
            x = x + velocity * delta_t * cos(theta);
            y = y + velocity * delta_t * sin(theta);
        } else {
            x = x + (velocity / yaw_rate) * (sin(theta + yaw_rate * delta_t) - sin(theta));
            y = y + (velocity / yaw_rate) * (cos(theta) - cos(theta + yaw_rate * delta_t));
            theta = theta + yaw_rate * delta_t;
        }
        // adding noise to prediction
        particles[i].x = x + N_x(gen);
        particles[i].y = y + N_y(gen);
        particles[i].theta = theta + N_theta(gen);
    }
}
    
void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
    for (int i = 0; i < observations.size(); i++){
        double best_dist = 9999;
        double best_id = -1;
        double obs_x = observations[i].x;
        double obs_y = observations[i].y;
        for (int j = 0; j < predicted.size(); j++){
            double pred_x = predicted[j].x;
            double pred_y = predicted[j].y;
            double pred_id = predicted[j].id;
            double curr_dist = dist(obs_x, obs_y, pred_x, pred_y);
            if (curr_dist < best_dist){
                best_dist = curr_dist;
                best_id = pred_id;
            }
        }
        observations[i].id = best_id;
    }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
    double std_x = std_landmark[0];
    double std_y = std_landmark[1];
    double std_xx = std_x * std_x;
    double std_yy = std_y * std_y;
    double norm = 1.0 / (2.0 * M_PI * std_x * std_y);
    // generating noise
    default_random_engine gen;
    normal_distribution<double> N_x(0, std_x);
    normal_distribution<double> N_y(0, std_y);
    // iterate over the particles
    double part_norm = 0.0;
    for (int i = 0; i < particles.size(); i++){
        double part_x = particles[i].x;
        double part_y = particles[i].y;
        double part_theta = particles[i].theta;
        // convert observations to map coordinates
        vector<LandmarkObs> trans_obs(observations.size());
        for (int j = 0; j < observations.size(); j++){
            trans_obs[j].id = j;
            trans_obs[j].x = cos(part_theta) * observations[j].x - sin(part_theta) * observations[j].y + part_x;
            trans_obs[j].y = sin(part_theta) * observations[j].x + cos(part_theta) * observations[j].y + part_y;
        }
        // filter landmarks in sensor range
        vector<LandmarkObs> pred_lm;
        for (int j = 0; j < map_landmarks.landmark_list.size(); j++){
            Map::single_landmark_s curr_lm = map_landmarks.landmark_list[j];
            if ((fabs(part_x - curr_lm.x_f) <= sensor_range) && (fabs(part_y - curr_lm.y_f) <= sensor_range)){
                pred_lm.push_back({curr_lm.id_i, curr_lm.x_f, curr_lm.y_f});
            }
        }
        // using nearest neighbour algorithm
        dataAssociation(pred_lm, trans_obs);
        // reset weight
        particles[i].weight = 1.0;
        // update weights
        for (int j = 0; j < trans_obs.size(); j++){
            for (int k = 0; k < pred_lm.size(); k++){
                if (trans_obs[j].id == pred_lm[k].id){
                    particles[i].weight *= norm * exp(-1.0 * ((pow((trans_obs[j].x - pred_lm[k].x), 2) / (2.0 * std_xx)) + (pow((trans_obs[j].y - pred_lm[k].y), 2) / (2.0 * std_yy))));
                }
            }
        }
        part_norm += particles[i].weight;
    }
    // normalize weights
    for (int i = 0; i < particles.size(); i++) {
        particles[i].weight /= part_norm;
        weights[i] = particles[i].weight;
    }
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
    vector<Particle> resamp_part;
    // generate random index
    default_random_engine gen;
    uniform_int_distribution<int> part_index(0, num_particles - 1);
    int curr_index = part_index(gen);
    // resample
    double beta = 0.0;
    double max_weight_2 = 2.0 * *max_element(weights.begin(), weights.end());
    for (int i = 0; i < particles.size(); i++) {
        uniform_real_distribution<double> random_weight(0.0, max_weight_2);
        beta += random_weight(gen);
        while (beta > weights[curr_index]) {
            beta -= weights[curr_index];
            curr_index = (curr_index + 1) % num_particles;
        }
        resamp_part.push_back(particles[curr_index]);
    }
    particles = resamp_part;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates

    particle.associations.clear();
    particle.sense_x.clear();
    particle.sense_y.clear();
    
    particle.associations = associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
    
    return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
