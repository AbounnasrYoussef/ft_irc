#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <iostream>

class Channel {
	private:
		std::string _name;
	public:
		Channel();
		Channel(const std::string& name);
		Channel(const Channel& other);
		Channel& operator=(const Channel& other);
		~Channel();
		// getters/setters...
};
#endif