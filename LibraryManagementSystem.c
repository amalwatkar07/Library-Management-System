#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIBRARIANS 10
#define MAX_ADMINS 5
#define MAX_BORROWED 5
#define MAX_RESERVATIONS 10
#define BORROW_LIMIT_DAYS 15

// ================= STRUCTURES =================

typedef struct BookCopy {
    char copyID[20];
    struct BookCopy *next;
} BookCopy;

typedef struct Book {
    char title[100];
    char author[100];
    char genre[50];
    char bookID[20];
    int availability;
    BookCopy *copies;
    struct Book *next;
} Book; 

typedef struct BorrowedBook {
    char title[100]; // Added title for easier tracking
    char copyID[20];
    int borrowDay, borrowMonth, borrowYear;
    int returnDay, returnMonth, returnYear;
    struct BorrowedBook *next;
} BorrowedBook; 

typedef struct Reservation {
    char rollNo[20];
    struct Reservation *next;
} Reservation; 

typedef struct ReservationQueue {
    char bookID[20];
    Reservation *front, *rear;
    struct ReservationQueue *next;
} ReservationQueue; 

typedef struct Student {
    char name[100];
    char rollNo[20];
    char password[20];
    char contact[20];
    BorrowedBook *borrowedHead;
    struct Student *next;
} Student;

typedef struct Librarian {
    char name[100];
    char id[20];
    char password[20];
    char contact[20];
    BorrowedBook *borrowedHead;
} Librarian; 

typedef struct Admin {
    char name[100];
    char id[20];
    char password[20];
    char contact[20];
    BorrowedBook *borrowedHead;
} Admin;

// ============== LINKED LIST INITIALIZATIONS ============== 

Book *bookHead = NULL;
Student *studentHead = NULL;
Librarian librarianList[MAX_LIBRARIANS];
Admin adminList[MAX_ADMINS];
int librarianCount = 0, adminCount = 0;
ReservationQueue *reservationQueueHead = NULL; 

// ================= FUNCTION DECLARATIONS ================= 

// Utility
void trimNewline(char *str);
int dateDifference(int d1, int m1, int y1, int d2, int m2, int y2); 

// Book Management
void addBook();
void removeBook();
void displayBooks();
Book* searchBookByID(char *bookID);
Book* searchBookByTitle(char *title);

// Borrow & Return
void borrowBook(Student *student);
void returnBook(Student *student);
void viewAllBorrowedBooks();

// Reservations
void reserveBook(char *bookID, char *rollNo);
void processReservation(char *bookID);
void viewAllReservations(); 

// Login & Dashboard
void loginStudent();
void loginLibrarian();
void loginAdmin();
void registerStudent();
void registerLibrarian();
void registerAdmin();
void studentDashboard(Student *s);
void librarianDashboard(Librarian *lib);
void adminDashboard(Admin *adm); 

// Admin/Management
void editStudentDetails();
void viewStudentDetails();

// Menu
void showMainMenu();

// ================= FUNCTION DEFINITIONS ================= 

// Utility
void trimNewline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

int dateDifference(int d1, int m1, int y1, int d2, int m2, int y2) {
    // Convert both dates to days since a reference date
    int days1 = y1 * 365 + m1 * 30 + d1;  // Approximation: every month has 30 days
    int days2 = y2 * 365 + m2 * 30 + d2;
    // Difference between the two dates
    return days2 - days1;
}

// Book Management
void addBook() {
    Book *newBook = (Book *)malloc(sizeof(Book));

    printf("Enter Book Title: ");
    fgets(newBook->title, sizeof(newBook->title), stdin);
    trimNewline(newBook->title);  // Remove the newline character

    printf("Enter Book Author: ");
    fgets(newBook->author, sizeof(newBook->author), stdin);
    trimNewline(newBook->author);

    printf("Enter Book Genre: ");
    fgets(newBook->genre, sizeof(newBook->genre), stdin);
    trimNewline(newBook->genre);

    printf("Enter Book ID: ");
    fgets(newBook->bookID, sizeof(newBook->bookID), stdin);
    trimNewline(newBook->bookID);

    printf("Enter Availability (1 for available, 0 for unavailable): ");
    scanf("%d", &newBook->availability);
    getchar();  // Consume the newline character left by scanf

    newBook->copies = NULL;  // Initialize with no copies
    newBook->next = bookHead;
    bookHead = newBook;

    printf("Book added successfully!\n");

}

void removeBook() {
    char bookID[20];
    printf("Enter the Book ID to remove: ");
    fgets(bookID, sizeof(bookID), stdin);
    trimNewline(bookID);

    Book *prev = NULL;
    Book *curr = bookHead;

    while (curr) {
        if (strcmp(curr->bookID, bookID) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                bookHead = curr->next;  // Remove the first book in the list
            }
            free(curr);
            printf("Book removed successfully!\n");
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("Book with ID %s not found.\n", bookID);
}

void displayBooks() {
    if (bookHead == NULL) {
        printf("No books available in the library.\n");
        return;
    }
 
    Book *curr = bookHead;
    while (curr) {
        printf("\nTitle: %s\nAuthor: %s\nGenre: %s\nBook ID: %s\nAvailability: %d\n",
                curr->title, curr->author, curr->genre, curr->bookID, curr->availability);
        curr = curr->next;
    }
}

Book* searchBookByID(char *bookID) {
    Book *curr = bookHead;
    while (curr) {
        if (strcmp(curr->bookID, bookID) == 0) {
            return curr;  // Book found
        }
        curr = curr->next;
    }
    return NULL;  // Book not found
}

Book* searchBookByTitle(char *title) {
    Book *curr = bookHead;
    while (curr) {
        if (strcmp(curr->title, title) == 0) {
            return curr;  // Book found
        }
        curr = curr->next;
    }
    return NULL;  // Book not found
}

// Borrow & Return
void borrowBook(Student *student) {
    char bookID[20];
    printf("Enter the Book ID to borrow: ");
    fgets(bookID, sizeof(bookID), stdin);
    trimNewline(bookID);

    Book *book = searchBookByID(bookID);
    if (book == NULL) {
        printf("Book not found.\n");
        return;
    }

    // Check if the student has reached the borrow limit
    if (student->borrowedHead != NULL) {
        int borrowedCount = 0;
        BorrowedBook *curr = student->borrowedHead;
        while (curr) {
            borrowedCount++;
            curr = curr->next;
        }
        if (borrowedCount >= MAX_BORROWED) {
            printf("You have reached the borrow limit.\n");
            return;
        }
    }

    // Book is available to borrow
    if (book->availability > 0) {
        BorrowedBook *newBorrowedBook = (BorrowedBook *)malloc(sizeof(BorrowedBook));
        printf("Enter borrow date (DD MM YYYY): ");
        scanf("%d %d %d", &newBorrowedBook->borrowDay, &newBorrowedBook->borrowMonth, &newBorrowedBook->borrowYear);
        getchar();  // Consume the newline

        newBorrowedBook->returnDay = newBorrowedBook->borrowDay + BORROW_LIMIT_DAYS;  // Set the return date
        newBorrowedBook->returnMonth = newBorrowedBook->borrowMonth;
        newBorrowedBook->returnYear = newBorrowedBook->borrowYear;
        strcpy(newBorrowedBook->title, book->title);
        strcpy(newBorrowedBook->copyID, book->bookID);  // Assuming one copy per book

        newBorrowedBook->next = student->borrowedHead;
        student->borrowedHead = newBorrowedBook;

        book->availability--;  // Decrease the availability
        printf("Book borrowed successfully!\n");
    } else {
        printf("Book is currently unavailable.\n");
    }
}

void returnBook(Student *student) {
    char bookID[20];
    printf("Enter the Book ID to return: ");
    fgets(bookID, sizeof(bookID), stdin);
    trimNewline(bookID);

    BorrowedBook *prev = NULL;
    BorrowedBook *curr = student->borrowedHead;
    while (curr) {
        if (strcmp(curr->copyID, bookID) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                student->borrowedHead = curr->next;  // If the book is the first one in the list
            }
            free(curr);

            // Find the book in the library and increase availability
            Book *book = searchBookByID(bookID);
            if (book) {
                book->availability++;
                printf("Book returned successfully!\n");
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("Book not found in your borrowed list.\n");
}

void viewAllBorrowedBooks() {
    if (studentHead == NULL) {
        printf("No students in the system.\n");
        return;
    }

    Student *studentCurr = studentHead;
    while (studentCurr) {
        BorrowedBook *borrowedCurr = studentCurr->borrowedHead;
        while (borrowedCurr) {
            printf("\nStudent: %s\nBook Title: %s\nBorrowed on: %d/%d/%d\nReturn by: %d/%d/%d\n",
                studentCurr->name, borrowedCurr->title, borrowedCurr->borrowDay, borrowedCurr->borrowMonth, borrowedCurr->borrowYear,
                borrowedCurr->returnDay, borrowedCurr->returnMonth, borrowedCurr->returnYear);
            borrowedCurr = borrowedCurr->next;
        }
        studentCurr = studentCurr->next;
    }
}

// Reservations
void reserveBook(char *bookID, char *rollNo) {
    Book *book = searchBookByID(bookID);
    if (book == NULL) {
        printf("Book not found.\n");
        return;
    }

    // Check if the book is available
    if (book->availability > 0) {
        printf("The book is available. No need to reserve.\n");
        return;
    }

    // Check if the reservation queue already exists for this book
    ReservationQueue *queue = reservationQueueHead;
    while (queue) {
        if (strcmp(queue->bookID, bookID) == 0) {
            // Add reservation to the existing queue
            Reservation *newReservation = (Reservation *)malloc(sizeof(Reservation));
            strcpy(newReservation->rollNo, rollNo);
            newReservation->next = queue->rear;
            queue->rear = newReservation;

            printf("Book reserved successfully.\n");
            return;
        }
        queue = queue->next;
    }

    // If no reservation queue exists for this book, create one
    ReservationQueue *newQueue = (ReservationQueue *)malloc(sizeof(ReservationQueue));
    strcpy(newQueue->bookID, bookID);
    newQueue->front = newQueue->rear = NULL;
    newQueue->next = reservationQueueHead;
    reservationQueueHead = newQueue;

    // Add the reservation to the new queue
    Reservation *newReservation = (Reservation *)malloc(sizeof(Reservation));
    strcpy(newReservation->rollNo, rollNo);
    newReservation->next = NULL;
    newQueue->front = newQueue->rear = newReservation;

    printf("Book reserved successfully.\n");
}

void processReservation(char *bookID) {
    ReservationQueue *queue = reservationQueueHead;
    while (queue) {
        if (strcmp(queue->bookID, bookID) == 0) {
            if (queue->front == NULL) {
                printf("No reservations for this book.\n");
                return;
            }

            Reservation *reservation = queue->front;
            // Process the first reservation
            Student *student = studentHead;
            while (student) {
                if (strcmp(student->rollNo, reservation->rollNo) == 0) {
                    // Allow the student to borrow the book
                   borrowBook(student);

                    // Remove the reservation from the queue
                    queue->front = reservation->next;
                    free(reservation);
                    printf("Reservation processed successfully. Book borrowed.\n");
                    return;
                }
                student = student->next;
            }
        }
        queue = queue->next;
    }
    printf("Reservation not found.\n");

}

void viewAllReservations() {
    if (reservationQueueHead == NULL) {
        printf("No reservations in the system.\n");
        return;
    }

    ReservationQueue *queue = reservationQueueHead;
    while (queue) {
        printf("Reservations for Book ID: %s\n", queue->bookID);
        Reservation *reservation = queue->front;
        while (reservation) {
            printf("\tStudent Roll No: %s\n", reservation->rollNo);
            reservation = reservation->next;
        }
        queue = queue->next;
    }
}

// Login & Dashboard
void loginStudent() {
    char rollNo[20], password[20];
    printf("Enter your Roll No: ");
    fgets(rollNo, sizeof(rollNo), stdin);
    trimNewline(rollNo);
 
    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    trimNewline(password);
 
    Student *curr = studentHead;
    while (curr) {
        if (strcmp(curr->rollNo, rollNo) == 0 && strcmp(curr->password, password) == 0) {
            studentDashboard(curr);
            return;
        }
        curr = curr->next;
    }
 
    printf("Invalid Roll No. or password.\n");
    char choice;
    printf("Do you want to register? (Y/N): ");
    scanf(" %c", &choice);
    getchar();
    if (choice == 'Y' || choice == 'y') {
        registerStudent();
    }
}

void loginLibrarian() {
    char id[20], password[20];
    printf("Enter your Librarian ID: ");
    fgets(id, sizeof(id), stdin);
    trimNewline(id);

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    trimNewline(password);

    for (int i = 0; i < MAX_LIBRARIANS; i++) {
        if (strcmp(librarianList[i].id, id) == 0 && strcmp(librarianList[i].password, password) == 0) {
            librarianDashboard(&librarianList[i]);
            return;
        }
    }

    printf("Invalid ID or password.\n");
    char choice;
    printf("Do you want to register? (Y/N): ");
    scanf(" %c", &choice);
    getchar();
    if (choice == 'Y' || choice == 'y') {
        registerLibrarian();
    }
}

void loginAdmin() {
    char id[20], password[20];
    printf("Enter your Admin ID: ");
    fgets(id, sizeof(id), stdin);
    trimNewline(id);

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    trimNewline(password);

    for (int i = 0; i < MAX_ADMINS; i++) {
        if (strcmp(adminList[i].id, id) == 0 && strcmp(adminList[i].password, password) == 0) {
            adminDashboard(&adminList[i]);
            return;
        }
    }

    printf("Invalid ID or password.\n");
    char choice;
    printf("Do you want to register? (Y/N): ");
    scanf(" %c", &choice);
    getchar();
    if (choice == 'Y' || choice == 'y') {
        registerAdmin();
    }
}

void registerStudent() {
    Student *newStudent = (Student *)malloc(sizeof(Student));
    printf("Enter your name: ");
    fgets(newStudent->name, sizeof(newStudent->name), stdin);
    trimNewline(newStudent->name);

    printf("Enter your Roll No: ");
    fgets(newStudent->rollNo, sizeof(newStudent->rollNo), stdin);
    trimNewline(newStudent->rollNo);

    printf("Enter your password: ");
    fgets(newStudent->password, sizeof(newStudent->password), stdin);
    trimNewline(newStudent->password);

    printf("Enter your contact number: ");
    fgets(newStudent->contact, sizeof(newStudent->contact), stdin);
    trimNewline(newStudent->contact);

    newStudent->borrowedHead = NULL;
    newStudent->next = studentHead;
    studentHead = newStudent;

    printf("Registration successful! You can now log in.\n");
}

void registerLibrarian() {
    if (librarianCount >= MAX_LIBRARIANS) {
        printf("Maximum number of librarians reached.\n");
        return;
    }

    Librarian *lib = &librarianList[librarianCount];

    printf("Enter Name: ");
    fgets(lib->name, sizeof(lib->name), stdin);
    trimNewline(lib->name);

    printf("Enter ID: ");
    fgets(lib->id, sizeof(lib->id), stdin);
    trimNewline(lib->id);

    printf("Enter Password: ");
    fgets(lib->password, sizeof(lib->password), stdin);
    trimNewline(lib->password);

    printf("Enter Contact: ");
    fgets(lib->contact, sizeof(lib->contact), stdin);
    trimNewline(lib->contact);

    librarianCount++;
    printf("Librarian registered successfully!\n");
}

void registerAdmin() {
    if (adminCount >= MAX_ADMINS) {
        printf("Maximum number of admins reached.\n");
        return;
    }

    Admin *adm = &adminList[adminCount];

    printf("Enter Name: ");
    fgets(adm->name, sizeof(adm->name), stdin);
    trimNewline(adm->name);

    printf("Enter ID: ");
    fgets(adm->id, sizeof(adm->id), stdin);
    trimNewline(adm->id);

    printf("Enter Password: ");
    fgets(adm->password, sizeof(adm->password), stdin);
    trimNewline(adm->password);

    printf("Enter Contact: ");
    fgets(adm->contact, sizeof(adm->contact), stdin);
    trimNewline(adm->contact);

    adminCount++;
    printf("Admin registered successfully!\n");
}

void studentDashboard(Student *s) {
    int choice;
    do {
        printf("\nWelcome %s! Choose an option:\n", s->name);
        printf("1. Display available books\n");
        printf("2. Search for a book by Title\n");
        printf("3. Borrow a book\n");
        printf("4. Return a book\n");
        printf("5. View borrowed books\n");
        printf("6. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: displayBooks(); break;
            case 2: char t[20]; printf("\nEnter the Title to search: "); fgets(t, sizeof(t), stdin); trimNewline(t); Book* found = searchBookByTitle(t); if (found) printf("Book Found: %s by %s\n", found->title, found->author); else printf("Book not found.\n"); break;
            case 3: borrowBook(s); break;
            case 4: returnBook(s); break;
            case 5: viewAllBorrowedBooks(); break;
            case 6: printf("Logging out...\n"); break;
            default: printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 6);
}

void librarianDashboard(Librarian *lib) {
    int choice;
    do {
        printf("\nWelcome Librarian %s! Choose an option:\n", lib->name);
        printf("1. Display available books\n");
        printf("2. Search for a book by Title\n");
        printf("3. Search for a book by ID\n");
        printf("4. Add a new book\n");
        printf("5. Remove a book\n");
        printf("6. View student details\n");
        printf("7. View all borrowed books\n");
        printf("8. View reservations\n");
        printf("9. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: displayBooks(); break;
            case 2: char t[20]; printf("\nEnter the Title to search: "); fgets(t, sizeof(t), stdin); trimNewline(t); Book* found = searchBookByTitle(t); if (found) printf("Book Found: %s by %s\n", found->title, found->author); else printf("Book not found.\n"); break;
            case 3: char i[20]; printf("\nEnter the ID to search: "); fgets(i, sizeof(i), stdin); trimNewline(i); Book* foundd = searchBookByID(i); if (foundd) printf("Book Found: %s by %s\n", foundd->title, foundd->author); else printf("Book not found.\n"); break;
            case 4: addBook(); break;
            case 5: removeBook(); break;
            case 6: viewStudentDetails(); break;
            case 7: viewAllBorrowedBooks(); break;
            case 8: viewAllReservations(); break;
            case 9: printf("Logging out...\n"); break;
            default: printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 9);
}

void adminDashboard(Admin *adm) {
    int choice;
    do {
        printf("\nWelcome Admin %s! Choose an option:\n", adm->name);
        printf("1. Display available books\n");
        printf("2. Edit student details\n");
        printf("3. View all borrowed books\n");
        printf("4. View reservations\n");
        printf("5. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: displayBooks(); break;
            case 2: editStudentDetails(); break;
            case 3: viewAllBorrowedBooks(); break;
            case 4: viewAllReservations(); break;
            case 5: printf("Logging out...\n"); break;
            default: printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 5);
}

// Admin/Management
void editStudentDetails() {
    char rollNo[20];
    printf("Enter the Roll No of the student whose details you want to edit: ");
    fgets(rollNo, sizeof(rollNo), stdin);
    trimNewline(rollNo);

    Student *curr = studentHead;
    while (curr) {
        if (strcmp(curr->rollNo, rollNo) == 0) {
            printf("Student found! Editing details...\n");
            printf("Enter new name (or press Enter to skip): ");
            fgets(curr->name, sizeof(curr->name), stdin);
            trimNewline(curr->name);
            printf("Enter new contact number (or press Enter to skip): ");
            fgets(curr->contact, sizeof(curr->contact), stdin);
            trimNewline(curr->contact);
            printf("Student details updated!\n");
            return;
        }
        curr = curr->next;
    }
    printf("Student with Roll No %s not found.\n", rollNo);
}

void viewStudentDetails() {
    printf("\nStudent Details:\n");
    Student *curr = studentHead;
    if (!curr) {
        printf("No students found.\n");
        return;
    }

    while (curr) {
        printf("\nName: %s\nRoll No: %s\nContact: %s\n", curr->name, curr->rollNo, curr->contact);

        // Display borrowed books
        BorrowedBook *borrowedCurr = curr->borrowedHead;
        if (borrowedCurr) {
            printf("Borrowed Books:\n");
            while (borrowedCurr) {
                printf("Title: %s, Copy ID: %s, Borrowed on: %d/%d/%d\n", borrowedCurr->title, borrowedCurr->copyID, borrowedCurr->borrowDay, borrowedCurr->borrowMonth, borrowedCurr->borrowYear);
                borrowedCurr = borrowedCurr->next;
            }
        } else {
            printf("No borrowed books.\n");
        }

        curr = curr->next;
    }
}

// Menu
void showMainMenu() {
    int choice;
    while (1) {
        printf("\n===== LIBRARY MANAGEMENT SYSTEM =====\n");
        printf("1. Student Login\n");
        printf("2. Librarian Login\n");
        printf("3. Admin Login\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline left by scanf

        switch (choice) {
            case 1:
                loginStudent();
                break;
            case 2:
                loginLibrarian();
                break;
            case 3:
                loginAdmin();
                break;
            case 4:
                printf("Exiting system. Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

// ================= MAIN =================

int main() {
    showMainMenu();
    return 0;
}