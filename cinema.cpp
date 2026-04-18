#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

class Show {
public:
    std::string time;
    double price;
    std::vector<std::vector<char>> seats;

    Show(std::string t, double p, int rows, int cols)
        : time(t), price(p) {
        seats.resize(rows, std::vector<char>(cols, 'A'));
    }

    Show() : price(0.0) {}

    void save(std::ofstream& ofs) const {
        int rows = (int)seats.size();
        int cols = (rows > 0) ? (int)seats[0].size() : 0; // FIX: guard against empty seats
        ofs << time << " " << price << " " << rows << " " << cols << " ";
        for (const auto& row : seats) {
            for (char seat : row) {
                ofs << seat;
            }
        }
    }

    void load(std::ifstream& ifs) {
        int rows, cols;
        ifs >> time >> price >> rows >> cols;
        seats.resize(rows, std::vector<char>(cols));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                ifs >> seats[i][j];
            }
        }
    }

    void displaySeating() const {
        int cols = (int)seats[0].size();
        std::cout << "  ";
        for (int i = 0; i < cols; ++i) {
            std::cout << std::setw(2) << (i + 1);
        }
        std::cout << "\n";
        for (int i = 0; i < (int)seats.size(); ++i) { // FIX: use int to avoid signed/unsigned warning
            std::cout << (char)('A' + i) << " ";
            for (char seat : seats[i]) {
                std::cout << std::setw(2) << seat;
            }
            std::cout << "\n";
        }
    }
};

class Movie {
public:
    std::string title;
    std::string genre;
    int duration;
    std::vector<Show> showtimes;

    Movie(std::string t, std::string g, int d)
        : title(t), genre(g), duration(d) {}

    Movie() : duration(0) {}

    void save(std::ofstream& ofs) const {
        ofs << title << "|" << genre << "|" << duration << "|" << showtimes.size() << "|";
        for (const auto& show : showtimes) {
            show.save(ofs);
            ofs << "|";
        }
        ofs << "\n";
    }

    void load(std::ifstream& ifs) {
        std::getline(ifs, title, '|');
        std::getline(ifs, genre, '|');
        ifs >> duration;
        ifs.ignore();
        int numShows;
        ifs >> numShows;
        ifs.ignore();
        showtimes.clear();
        for (int i = 0; i < numShows; ++i) {
            Show show;
            show.load(ifs);
            showtimes.push_back(show);
            ifs.ignore();
        }
        ifs.ignore();
    }
};

class MovieTicketSystem {
private:
    std::vector<Movie> movies;
    const std::string dataFile = "movies.dat";

public: // FIX: moved getIntInput and getDoubleInput to public so main() can access them
    int getIntInput() {
        int value;
        while (!(std::cin >> value)) {
            std::cout << "Invalid input. Please enter a number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }

    double getDoubleInput() {
        double value;
        while (!(std::cin >> value)) {
            std::cout << "Invalid input. Please enter a number: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }

    MovieTicketSystem() {
        loadData();
    }

    ~MovieTicketSystem() {
        saveData();
    }

    void addMovie() {
        clearScreen();
        std::cout << "--- Add New Movie ---\n";
        std::string title, genre;
        int duration;
        std::cout << "Enter movie title: ";
        std::getline(std::cin, title);
        std::cout << "Enter genre: ";
        std::getline(std::cin, genre);
        std::cout << "Enter duration (in minutes): ";
        duration = getIntInput();
        movies.push_back(Movie(title, genre, duration));
        std::cout << "Movie '" << title << "' added successfully.\n";
    }

    void addShowtime() {
        clearScreen();
        if (movies.empty()) {
            std::cout << "No movies available. Please add a movie first.\n";
            return;
        }

        std::cout << "--- Add New Showtime ---\n";
        listMovies(); // FIX: call listMovies() instead of viewAllMovies() to avoid double clearScreen
        std::cout << "Enter the title of the movie to add a showtime for: ";
        std::string title;
        std::getline(std::cin, title);

        auto it = std::find_if(movies.begin(), movies.end(), [&](const Movie& m) {
            return m.title == title;
        });

        if (it != movies.end()) {
            std::string time;
            double price;
            int rows, cols;
            std::cout << "Enter showtime (e.g., 10:00 AM): ";
            std::getline(std::cin, time);
            std::cout << "Enter ticket price: ";
            price = getDoubleInput();
            std::cout << "Enter number of seat rows: ";
            rows = getIntInput();
            std::cout << "Enter number of seat columns: ";
            cols = getIntInput();
            it->showtimes.push_back(Show(time, price, rows, cols));
            std::cout << "Showtime added successfully for '" << title << "'.\n";
        } else {
            std::cout << "Movie not found.\n";
        }
    }

    // FIX: extracted a version that does NOT call clearScreen, for internal use
    void listMovies() const {
        std::cout << "--- Current Movies ---\n";
        if (movies.empty()) {
            std::cout << "No movies available.\n";
            return;
        }
        for (const auto& movie : movies) {
            std::cout << "Title: " << movie.title << "\n";
            std::cout << "  Genre: " << movie.genre << "\n";
            std::cout << "  Duration: " << movie.duration << " minutes\n";
            if (!movie.showtimes.empty()) {
                std::cout << "  Showtimes:\n";
                for (int i = 0; i < (int)movie.showtimes.size(); ++i) { // FIX: signed/unsigned
                    std::cout << "    " << i + 1 << ". " << movie.showtimes[i].time
                        << " ($" << std::fixed << std::setprecision(2) << movie.showtimes[i].price << ")\n";
                }
            } else {
                std::cout << "  No showtimes scheduled.\n";
            }
            std::cout << "-----------------------\n";
        }
    }

    void viewAllMovies() const {
        clearScreen();
        listMovies(); // FIX: reuse listMovies() here
    }

    void bookTickets() {
        clearScreen();
        if (movies.empty()) {
            std::cout << "No movies available to book tickets for.\n";
            return;
        }
        listMovies(); // FIX: use listMovies() to avoid unintended clearScreen
        std::cout << "--- Book Tickets ---\n";
        std::cout << "Enter the title of the movie you want to watch: ";
        std::string movieTitle;
        std::getline(std::cin, movieTitle);

        auto movieIt = std::find_if(movies.begin(), movies.end(), [&](const Movie& m) {
            return m.title == movieTitle;
        });

        if (movieIt == movies.end()) {
            std::cout << "Movie not found.\n";
            return;
        }

        if (movieIt->showtimes.empty()) {
            std::cout << "No showtimes available for this movie.\n";
            return;
        }

        std::cout << "Available showtimes for " << movieIt->title << ":\n";
        for (int i = 0; i < (int)movieIt->showtimes.size(); ++i) { // FIX: signed/unsigned
            std::cout << "  " << i + 1 << ". " << movieIt->showtimes[i].time
                << " - Price: $" << std::fixed << std::setprecision(2) << movieIt->showtimes[i].price << "\n";
        }
        std::cout << "Enter the showtime number: ";
        int showtimeIndex = getIntInput();

        if (showtimeIndex < 1 || showtimeIndex > (int)movieIt->showtimes.size()) { // FIX: cast to int
            std::cout << "Invalid showtime number.\n";
            return;
        }

        Show& selectedShow = movieIt->showtimes[showtimeIndex - 1];
        std::cout << "\nSeating for " << movieIt->title << " at " << selectedShow.time << ":\n";
        selectedShow.displaySeating();

        int numTickets;
        std::cout << "How many tickets do you want to book? ";
        numTickets = getIntInput();

        if (numTickets <= 0) {
            std::cout << "Invalid number of tickets.\n";
            return;
        }

        std::vector<std::pair<int, int>> selectedSeats;
        for (int i = 0; i < numTickets; ++i) {
            char rowChar;
            int col;
            std::cout << "Enter seat " << i + 1 << " (e.g., A 5): ";
            std::cin >> rowChar >> col;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            int row = toupper(rowChar) - 'A';
            col--;

            if (row < 0 || row >= (int)selectedShow.seats.size() ||  // FIX: cast to int
                col < 0 || col >= (int)selectedShow.seats[0].size()) {
                std::cout << "Invalid seat location. Please try again.\n";
                i--;
                continue;
            }

            if (selectedShow.seats[row][col] == 'B') {
                std::cout << "Seat is already booked. Please choose another one.\n";
                i--;
                continue;
            }

            selectedSeats.push_back({row, col});
            selectedShow.seats[row][col] = 'B';
        }

        double totalCost = numTickets * selectedShow.price;
        std::cout << "\n--- Booking Confirmation ---\n";
        std::cout << "Movie: " << movieIt->title << "\n";
        std::cout << "Showtime: " << selectedShow.time << "\n";
        std::cout << "Tickets: " << numTickets << "\n";
        std::cout << "Seats: ";
        for (const auto& seat : selectedSeats) {
            std::cout << (char)('A' + seat.first) << seat.second + 1 << " ";
        }
        std::cout << "\nTotal Cost: $" << std::fixed << std::setprecision(2) << totalCost << "\n";
        std::cout << "Booking successful! Enjoy the movie.\n";
    }

    void saveData() const {
        std::ofstream ofs(dataFile);
        if (!ofs.is_open()) {
            std::cerr << "Error: Could not open file for saving data.\n";
            return;
        }
        ofs << movies.size() << "\n";
        for (const auto& movie : movies) {
            movie.save(ofs);
        }
        ofs.close();
    }

    void loadData() {
        std::ifstream ifs(dataFile);
        if (!ifs.is_open()) {
            return;
        }
        int numMovies;
        ifs >> numMovies;
        ifs.ignore();
        for (int i = 0; i < numMovies; ++i) {
            Movie movie;
            movie.load(ifs);
            movies.push_back(movie);
        }
        ifs.close();
    }
};

int main() {
    MovieTicketSystem ticketSystem; // FIX: renamed from 'system' to avoid shadowing std::system
    int choice;

    do {
        clearScreen();
        std::cout << "--- Movie Ticket Management System ---\n";
        std::cout << "1. Add a new movie\n";
        std::cout << "2. Add a showtime for a movie\n";
        std::cout << "3. View all movies and showtimes\n";
        std::cout << "4. Book tickets\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";
        choice = ticketSystem.getIntInput(); // FIX: now works since getIntInput is public

        switch (choice) {
            case 1:
                ticketSystem.addMovie();
                break;
            case 2:
                ticketSystem.addShowtime();
                break;
            case 3:
                ticketSystem.viewAllMovies();
                break;
            case 4:
                ticketSystem.bookTickets();
                break;
            case 5:
                std::cout << "Exiting system. Thank you!\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
        if (choice != 5) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (choice != 5);

    return 0;
}