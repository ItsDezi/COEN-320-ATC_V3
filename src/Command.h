#ifndef Command_H_
#define Command_H_
struct Command {
	int ID;
	int readOrChange; //0 = read, 1 = change
	int attributes; // 1 = x speed, 2 = y speed, 3 = z speed
	int newValue; // New value to set for the attribute (only applicable when readOrChange is 1)
};

#endif
