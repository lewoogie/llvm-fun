#include <iostream>   // For input/output streams
#include <string>     // For using std::string
#include <cstdlib>    // For exit()

// Function to write (output) the result of a calculation
// This function takes an integer value 'v' and prints it to the console.
void calc_write(int v)
{
    // Output the result using std::cout
    std::cout << "The result is: " << v << std::endl;
}

// Function to read an integer value from the user input
// This function takes a string 's' (which is the name of the variable)
// and prompts the user to input a value for it.
int calc_read(const std::string &s)
{
    std::string buf;  // String to store user input
    int val;          // Variable to store the parsed integer

    // Prompt the user to enter a value for the variable represented by 's'.
    std::cout << "Enter a value for " << s << ": ";
    
    // Get a line of input from the user
    std::getline(std::cin, buf);
    
    // Try to convert the input string to an integer
    try {
        val = std::stoi(buf);  // Convert string to integer
    } catch (const std::invalid_argument&) {
        // If conversion fails (invalid input), print an error message and exit
        std::cerr << "Invalid input: " << buf << std::endl;
        std::exit(1);  // Exit with status 1 indicating error
    }

    // Return the valid parsed integer
    return val;
}

int main() {
    // Example usage
    int x = calc_read("x");
    calc_write(x + 10);  // Just an example of adding 10 to the input value
    return 0;
}
