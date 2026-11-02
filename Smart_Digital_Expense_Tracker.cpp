#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <limits>
using namespace std;

struct Expense {
    int id;
    string username;
    string date;
    string category;
    string description;
    double amount;
};

const string USER_FILE = "users.txt";
const string EXPENSE_FILE = "expenses.txt";
const string BUDGET_FILE = "budgets.txt";

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin >> ws, value);
    return value;
}

double readAmount(const string& prompt) {
    double amount;
    while (true) {
        cout << prompt;
        if (cin >> amount && amount >= 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return amount;
        }
        cout << "Invalid amount. Enter a non-negative number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readInteger(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

vector<Expense> loadExpenses() {
    vector<Expense> expenses;
    ifstream file(EXPENSE_FILE);
    Expense expense;
    string line;

    while (getline(file, line)) {
        stringstream stream(line);
        string id, amount;
        if (getline(stream, id, '|') &&
            getline(stream, expense.username, '|') &&
            getline(stream, expense.date, '|') &&
            getline(stream, expense.category, '|') &&
            getline(stream, expense.description, '|') &&
            getline(stream, amount, '|')) {
            try {
                expense.id = stoi(id);
                expense.amount = stod(amount);
                expenses.push_back(expense);
            } catch (...) {
            }
        }
    }
    return expenses;
}

void saveExpenses(const vector<Expense>& expenses) {
    ofstream file(EXPENSE_FILE);
    for (const Expense& expense : expenses) {
        file << expense.id << '|'
             << expense.username << '|'
             << expense.date << '|'
             << expense.category << '|'
             << expense.description << '|'
             << fixed << setprecision(2) << expense.amount << "|\n";
    }
}

int nextExpenseId(const vector<Expense>& expenses) {
    int id = 1;
    for (const Expense& expense : expenses)
        id = max(id, expense.id + 1);
    return id;
}

bool registerUser() {
    string username = readLine("Create username: ");
    string password = readLine("Create password: ");

    ifstream input(USER_FILE);
    string savedUsername, savedPassword;
    while (input >> savedUsername >> savedPassword) {
        if (savedUsername == username) {
            cout << "Username already exists.\n";
            return false;
        }
    }

    ofstream output(USER_FILE, ios::app);
    output << username << ' ' << password << '\n';
    cout << "Registration successful.\n";
    return true;
}

bool loginUser(string& currentUser) {
    string username = readLine("Username: ");
    string password = readLine("Password: ");

    ifstream file(USER_FILE);
    string savedUsername, savedPassword;
    while (file >> savedUsername >> savedPassword) {
        if (savedUsername == username && savedPassword == password) {
            currentUser = username;
            cout << "Login successful. Welcome, " << username << ".\n";
            return true;
        }
    }

    cout << "Invalid username or password.\n";
    return false;
}

double getMonthlyBudget(const string& username, const string& month) {
    ifstream file(BUDGET_FILE);
    string savedUser, savedMonth;
    double budget;
    while (file >> savedUser >> savedMonth >> budget) {
        if (savedUser == username && savedMonth == month)
            return budget;
    }
    return 0;
}

void setMonthlyBudget(const string& username) {
    string month = readLine("Enter month (YYYY-MM): ");
    double budget = readAmount("Enter monthly budget: ");

    ifstream input(BUDGET_FILE);
    vector<string> lines;
    string savedUser, savedMonth;
    double savedBudget;
    bool updated = false;

    while (input >> savedUser >> savedMonth >> savedBudget) {
        if (savedUser == username && savedMonth == month) {
            savedBudget = budget;
            updated = true;
        }
        ostringstream row;
        row << savedUser << ' ' << savedMonth << ' '
            << fixed << setprecision(2) << savedBudget;
        lines.push_back(row.str());
    }

    if (!updated) {
        ostringstream row;
        row << username << ' ' << month << ' '
            << fixed << setprecision(2) << budget;
        lines.push_back(row.str());
    }

    ofstream output(BUDGET_FILE);
    for (const string& line : lines)
        output << line << '\n';

    cout << "Monthly budget saved.\n";
}

double monthlyTotal(const vector<Expense>& expenses,
                    const string& username, const string& month) {
    double total = 0;
    for (const Expense& expense : expenses) {
        if (expense.username == username &&
            expense.date.size() >= 7 &&
            expense.date.substr(0, 7) == month)
            total += expense.amount;
    }
    return total;
}

void showBudgetStatus(const vector<Expense>& expenses,
                      const string& username, const string& month) {
    double budget = getMonthlyBudget(username, month);
    double spent = monthlyTotal(expenses, username, month);

    cout << fixed << setprecision(2);
    cout << "Total spent: " << spent << '\n';

    if (budget <= 0) {
        cout << "No budget set for this month.\n";
        return;
    }

    cout << "Monthly budget: " << budget << '\n';
    cout << "Remaining budget: " << budget - spent << '\n';

    if (spent > budget)
        cout << "Warning: Monthly budget exceeded.\n";
    else if (spent >= budget * 0.8)
        cout << "Warning: You have used at least 80% of your budget.\n";
    else
        cout << "You are within your budget.\n";
}

void addExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    Expense expense;
    expense.id = nextExpenseId(expenses);
    expense.username = username;
    expense.date = readLine("Enter date (YYYY-MM-DD): ");
    expense.category = readLine("Enter category: ");
    expense.description = readLine("Enter description: ");
    expense.amount = readAmount("Enter amount: ");

    replace(expense.category.begin(), expense.category.end(), '|', '/');
    replace(expense.description.begin(), expense.description.end(), '|', '/');

    expenses.push_back(expense);
    saveExpenses(expenses);

    cout << "Expense added successfully. Expense ID: "
         << expense.id << '\n';
    showBudgetStatus(expenses, username, expense.date.substr(0, 7));
}

void viewExpenseHistory(const string& username) {
    vector<Expense> expenses = loadExpenses();
    bool found = false;

    cout << left << setw(6) << "ID" << setw(14) << "Date"
         << setw(18) << "Category" << setw(25) << "Description"
         << "Amount\n";

    for (const Expense& expense : expenses) {
        if (expense.username == username) {
            cout << left << setw(6) << expense.id
                 << setw(14) << expense.date
                 << setw(18) << expense.category
                 << setw(25) << expense.description
                 << fixed << setprecision(2) << expense.amount << '\n';
            found = true;
        }
    }

    if (!found)
        cout << "No expenses found.\n";
}

void categoryWiseExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    vector<pair<string, double>> totals;

    for (const Expense& expense : expenses) {
        if (expense.username != username)
            continue;

        auto it = find_if(totals.begin(), totals.end(),
            [&](const pair<string, double>& item) {
                return item.first == expense.category;
            });

        if (it == totals.end())
            totals.push_back({expense.category, expense.amount});
        else
            it->second += expense.amount;
    }

    sort(totals.begin(), totals.end(),
         [](const auto& first, const auto& second) {
             return first.second > second.second;
         });

    if (totals.empty()) {
        cout << "No expenses found.\n";
        return;
    }

    cout << fixed << setprecision(2);
    for (const auto& item : totals)
        cout << item.first << ": " << item.second << '\n';
}

void monthlyReport(const string& username) {
    string month = readLine("Enter month (YYYY-MM): ");
    vector<Expense> expenses = loadExpenses();
    double total = 0;
    int count = 0;

    cout << fixed << setprecision(2);
    for (const Expense& expense : expenses) {
        if (expense.username == username &&
            expense.date.size() >= 7 &&
            expense.date.substr(0, 7) == month) {
            cout << expense.date << " | " << expense.category
                 << " | " << expense.description
                 << " | " << expense.amount << '\n';
            total += expense.amount;
            count++;
        }
    }

    cout << "Number of expenses: " << count << '\n';
    cout << "Monthly total: " << total << '\n';
    showBudgetStatus(expenses, username, month);
}

void predictExpenses(const string& username) {
    string month = readLine("Enter month (YYYY-MM): ");
    vector<Expense> expenses = loadExpenses();
    double spent = monthlyTotal(expenses, username, month);
    double budget = getMonthlyBudget(username, month);

    int days = readInteger("Enter total days in this month: ");
    int currentDay = readInteger("Enter current day of the month: ");

    if (days <= 0 || currentDay <= 0 || currentDay > days) {
        cout << "Invalid day values.\n";
        return;
    }

    double predicted = spent / currentDay * days;
    cout << fixed << setprecision(2);
    cout << "Current spending: " << spent << '\n';
    cout << "Predicted monthly spending: " << predicted << '\n';

    if (budget > 0 && predicted > budget)
        cout << "Prediction: You may exceed your monthly budget.\n";
    else if (budget > 0)
        cout << "Prediction: You are likely to stay within your budget.\n";
    else
        cout << "Set a monthly budget to receive a budget prediction warning.\n";
}

void searchExpense(const string& username) {
    string keyword = readLine("Enter date, category, or description to search: ");
    vector<Expense> expenses = loadExpenses();
    bool found = false;

    for (const Expense& expense : expenses) {
        if (expense.username == username &&
            (expense.date.find(keyword) != string::npos ||
             expense.category.find(keyword) != string::npos ||
             expense.description.find(keyword) != string::npos)) {
            cout << expense.id << " | " << expense.date << " | "
                 << expense.category << " | " << expense.description
                 << " | " << fixed << setprecision(2)
                 << expense.amount << '\n';
            found = true;
        }
    }

    if (!found)
        cout << "No matching expense found.\n";
}

void deleteExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    int id = readInteger("Enter expense ID to delete: ");
    auto oldSize = expenses.size();

    expenses.erase(remove_if(expenses.begin(), expenses.end(),
        [&](const Expense& expense) {
            return expense.username == username && expense.id == id;
        }), expenses.end());

    if (expenses.size() == oldSize) {
        cout << "Expense not found.\n";
        return;
    }

    saveExpenses(expenses);
    cout << "Expense deleted successfully.\n";
}

void expenseMenu(const string& username) {
    int choice;

    do {
        cout << "\n1. Add Daily Expense\n";
        cout << "2. View Expense History\n";
        cout << "3. Category-wise Expense\n";
        cout << "4. Monthly Expense Report\n";
        cout << "5. Set Monthly Budget\n";
        cout << "6. Budget Warning\n";
        cout << "7. Expense Prediction\n";
        cout << "8. Search Expense\n";
        cout << "9. Delete Expense\n";
        cout << "0. Logout\n";

        choice = readInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                addExpense(username);
                break;
            case 2:
                viewExpenseHistory(username);
                break;
            case 3:
                categoryWiseExpense(username);
                break;
            case 4:
                monthlyReport(username);
                break;
            case 5:
                setMonthlyBudget(username);
                break;
            case 6: {
                string month = readLine("Enter month (YYYY-MM): ");
                showBudgetStatus(loadExpenses(), username, month);
                break;
            }
            case 7:
                predictExpenses(username);
                break;
            case 8:
                searchExpense(username);
                break;
            case 9:
                deleteExpense(username);
                break;
            case 0:
                cout << "Logged out.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

int main() {
    int choice;
    string currentUser;

    do {
        cout << "\nSMART DIGITAL EXPENSE TRACKER\n";
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "0. Exit\n";

        choice = readInteger("Enter your choice: ");

        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                if (loginUser(currentUser))
                    expenseMenu(currentUser);
                break;
            case 0:
                cout << "Program closed.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
