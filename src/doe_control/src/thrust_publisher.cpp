#include "doe_control/thrust_publisher.hpp"

using std::placeholders::_1;

namespace doe_control
{
    ThrustPublisher::ThrustPublisher(const rclcpp::NodeOptions & options) :
    Node ("thrust_publisher", options),
        // The distance between center of gravity and thrusters
        x_lens_ (SUPPORTED_THRUSTERS, 0),
        y_lens_ (SUPPORTED_THRUSTERS, 0),
        z_lens_ (SUPPORTED_THRUSTERS, 0),
        // Thruster orientation and force vectors
        x_contribs_ (SUPPORTED_THRUSTERS, 0),
        y_contribs_ (SUPPORTED_THRUSTERS, 0),
        z_contribs_ (SUPPORTED_THRUSTERS, 0)
    {
        this->declare_parameter<int>("num_thrusters", num_thrusters_);
        if (num_thrusters_ > SUPPORTED_THRUSTERS)
        {
            RCLCPP_ERROR(this->get_logger(), "Number of thrusters is greater than supported thruster count");
        }

        this->declare_parameter<int>("bits_per_thruster", bits_per_thruster_);
        this->get_parameter("bits_per_thruster", bits_per_thruster_);

        this->declare_parameter<double>("max_fwd", max_fwd_);
        this->get_parameter("max_fwd", max_fwd_);
        max_fwd_ *= 9.807; // kgf to N
        this->declare_parameter<double>("max_rev", max_rev_);
        max_rev_ *= 9.807; // kgf to N
        this->get_parameter("max_rev", max_rev_);

        encode_levels_ = pow(2,bits_per_thruster_);

        std::string names[SUPPORTED_THRUSTERS] = {"thruster1", "thruster2", "thruster3", "thruster4", "thruster5", "thruster6", "thruster8", "thruster9", "thruster10"};

        for (int i = 0; i < SUPPORTED_THRUSTERS; i++)
        {
            this->declare_parameter(names[i]+".contrib.x", x_contribs_[i]);
            this->declare_parameter(names[i]+".contrib.y", y_contribs_[i]);
            this->declare_parameter(names[i]+".contrib.z", z_contribs_[i]);
            this->declare_parameter(names[i]+".lx", x_lens_[i]);
            this->declare_parameter(names[i]+".ly", y_lens_[i]);
            this->declare_parameter(names[i]+".lz", z_lens_[i]);

            this->get_parameter(names[i]+".contrib.x", x_contribs_[i]);
            this->get_parameter(names[i]+".contrib.y", y_contribs_[i]);
            this->get_parameter(names[i]+".contrib.z", z_contribs_[i]);
            this->get_parameter(names[i]+".lx", x_lens_[i]);
            this->get_parameter(names[i]+".ly", y_lens_[i]);
            this->get_parameter(names[i]+".lz", z_lens_[i]);
        }
        std::vector<std::vector<double>> alloc_vec =  createAllocMat();

        cv::Mat alloc_mat (alloc_vec.size(), alloc_vec[0].size(), CV_64FC1);
        for (int i = 0; i < alloc_mat.rows; i++)
            for (int j = 0; j < alloc_mat.cols; j++)
                alloc_mat.at<double>(i,j) = alloc_vec[i][j];

        // RCLCPP_INFO(this->get_logger(), "Allocation Matrix:\n%s", alloc_mat);


        cv::invert(alloc_mat, pinv_alloc_, cv::DECOMP_SVD);

        forces_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("output_forces", 10);

        signals_pub_ = this->create_publisher<std_msgs::msg::UInt32>("signals", 10);

        forces_sub_ = this->create_subscription<geometry_msgs::msg::Wrench>("input_forces", 10, std::bind(&ThrustPublisher::wrenchCallback, this, _1));

        RCLCPP_INFO(this->get_logger(), "Thrust Allocator succesfully started!");

    }

    // Subscriber callback, processes msgs of type wrench from topic
    void ThrustPublisher::wrenchCallback(const geometry_msgs::msg::Wrench::SharedPtr msg) const
    {
        double tau_arr[6] =
        {
            msg->force.x, msg->force.y, msg->force.z,
            msg->torque.x, msg->torque.y, msg->torque.z,
        };

        cv::Mat tau_mat (6, 1, CV_64F);
        for (int i = 0; i < 6; i++)
            tau_mat.at<double>(i, 0) = tau_arr[i];

        cv::Mat thrust_mat =  pinv_alloc_*tau_mat;

        std::vector<double> thrust;
        uint32_t signal = 0;
        for (int i = 0; i < num_thrusters_ ; i++)
        {
            double thruster_thrust = thrust_mat.at<double>(i,0);
            thrust.push_back(thruster_thrust);

            uint32_t t_level = forceToLevel(thruster_thrust);
            uint32_t t_bits = t_level << (bits_per_thruster_ * i);
            // std::cout << "thruster " << i << " " << thruster_thrust << " " << t_bits  << " " << (t_bits >> bits_per_thruster_ * i) << std::endl;
            signal |= t_bits;
        }
        // TEMP FIX: PLEASE CHANGE ME LATER
        // so that the teensy can distinguish between rubbish and real data
        signal |= 0b10101010000000000000000000000000;


        auto forces_msg = std_msgs::msg::Float64MultiArray();
        forces_msg.data = thrust;
        forces_pub_->publish(forces_msg);
        auto signal_msg = std_msgs::msg::UInt32();
        signal_msg.data = signal;
        signals_pub_->publish(signal_msg);
    }

    std::vector<double> ThrustPublisher::cross(std::vector<double> r,std::vector<double> F){
        std::vector<double> tau;
        tau.push_back(r[1]*F[2] - r[2]*F[1]);
        tau.push_back(r[2]*F[0] - r[0]*F[2]);
        tau.push_back(r[0]*F[1] - r[1]*F[0]);
        return tau;
    }

    std::vector<std::vector<double>> ThrustPublisher::createAllocMat(){
        std::vector<std::vector<double>> alloc_mat;
        for (int i = 0; i < num_thrusters_; i++){
            std::vector<double> F = {x_contribs_[i], y_contribs_[i], z_contribs_[i]};
            std::vector<double> r = {x_lens_[i], y_lens_[i], z_lens_[i]};
            std::vector<double> tau = cross(r, F);
            F.insert(F.end(), tau.begin(), tau.end());
            alloc_mat.push_back(F);
        }
        std::vector<std::vector<double>> alloc_mat_trans(6, std::vector<double>(num_thrusters_));
        for(int i = 0; i < num_thrusters_; ++i){
            for(int j = 0; j < 6; ++j){
                alloc_mat_trans[j][i]=alloc_mat[i][j];
            }
        }
        return alloc_mat_trans;
    }

    uint32_t ThrustPublisher::forceToLevel(double force) const{
        uint32_t t_level;
        if (force < 0.1 && force > -0.1)
        {
            force = 0;
        }
        if (force >= 0)
        {
            // levels 0 to 15 (0 is no force)
            t_level = (uint32_t) (std::ceil(std::min(force, max_fwd_) / max_fwd_ * (encode_levels_/2 - 1)));
        // turn to levels 16 to 31
        t_level += encode_levels_ / 2;
        }
        else
        {
            // levels 0 to 16 (0 is max rev, 16 is no force)
        t_level = (uint32_t) (16 - std::ceil(std::max(force, -max_rev_) / -max_rev_ * (encode_levels_/2)));
        }

        t_level &= ((uint32_t)pow(2, bits_per_thruster_) - 1);
        return t_level;
    }




} // namespace doe_control

int main(int argc, char * argv[])
{
  try {
    rclcpp::init(argc, argv);
    auto options = rclcpp::NodeOptions();
    rclcpp::spin(std::make_shared<doe_control::ThrustPublisher>(options));
    rclcpp::shutdown();
  } catch (rclcpp::exceptions::RCLError const&){} // during testing sometimes throws error
  return 0;
}

