#include <iostream>
#include "sqlite-amalgamation-3460000/sqlite-amalgamation-3460000/sqlite3.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Windows.h>

// Constants for text colors
const char* RESET = "\033[0m";
const char* BLACK_TEXT = "\033[30m";
const char* RED_TEXT = "\033[31m";
const char* GREEN_TEXT = "\033[32m";
const char* YELLOW_TEXT = "\033[33m";
const char* BLUE_TEXT = "\033[34m";
const char* MAGENTA_TEXT = "\033[35m";
const char* CYAN_TEXT = "\033[36m";
const char* WHITE_TEXT = "\033[37m";

struct Creature
{
    int tierTavern = 0;
    int hp = 0;
    int atk = 0;
    std::string description = "";
    std::string name = "";
    std::string type = "";
    bool bubble = false;
};

class SQLiteDB {
private:
    sqlite3* db;
    char* zErrMsg = 0;

    static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
        for (int i = 0; i < argc; i++) {
            std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
        }
        std::cout << std::endl;
        return 0;
    }

    static int myCallback(void* data, int argc, char** argv, char** azColName) {
        std::vector<Creature>* creatures = static_cast<std::vector<Creature>*>(data);

        Creature creature;

        for (int i = 0; i < argc; i++) {

            switch (i)
            {
                case 1:
                    creature.tierTavern = std::atoi(argv[i]);
                    break;
                case 2:
                    creature.hp = std::atoi(argv[i]);
                    break;
                case 3:
                    creature.atk = std::atoi(argv[i]);
                    break;
                case 4:
                    creature.description = argv[i];
                    break;
                case 5:
                    creature.name = argv[i];
                    break;
                case 6:
                    creature.type = argv[i];
                    break;
                default:
                    break;
            }
        }
        creatures->push_back(creature);
        return 0;
    }

    void ErrorHandling(int result) {
        if (result != SQLITE_OK || result != SQLITE_DONE && result != SQLITE_ROW) {
            std::cerr << "Error SQL: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_free(zErrMsg);
        }
    }

public:
    SQLiteDB(const std::string& dbName) {
        if (sqlite3_open(dbName.c_str(), &db)) {
            std::cerr << "Error when opening/creating a database: " << sqlite3_errmsg(db) << std::endl;
            db = nullptr;
        }
    }

    ~SQLiteDB() {
        if (db) {
            sqlite3_close(db);
        }
    }

    // Places 14 random creatures from the database into the vector
    void selectCreatures(std::vector<Creature>* creatures)
    {
        std::string sql = "SELECT * FROM creatures ORDER BY RANDOM() LIMIT 14;";
        int result = sqlite3_exec(db, sql.c_str(), myCallback, static_cast<void*>(creatures), &zErrMsg);
        ErrorHandling(result);
    }

    void executeQuery(const std::string& sql) {
        int result = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        ErrorHandling(result);
    }

    void executeQueryWithParams(const std::string& sql, const std::vector<std::string>& params) {
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
        ErrorHandling(rc);
        // Binding of parameters
        for (size_t i = 0; i < params.size(); i++) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }

        // Execution
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
            std::cerr << "Error in preparing a query: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt); 
    }
};

void removeFile() {
    std::filesystem::path file_to_remove = "test.db";

    try {
        bool removed = std::filesystem::remove(file_to_remove);
        if (removed) {
            std::cout << "File deleted." << std::endl;
        }
        else {
            std::cout << "File not found." << std::endl;
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error when deleting file: " << e.what() << std::endl;
    }
}

void printCreature(Creature& creature)
{
    std::cout
        << "Name: " << creature.name << std::endl
        << "Description: " << creature.description << std::endl
        << "Tier Tavern: " << creature.tierTavern << std::endl
        << "ATK: " << creature.atk << std::endl
        << "HP: " << creature.hp << std::endl
        << "Type: " << creature.type << std::endl 
        << std::boolalpha 
        << "Bubble: " << creature.bubble << std::endl
        << std::noboolalpha
        << std::endl;
}


/// This function for parsing from file
/// TODO: add reading of numeric fields of a file
//std::vector<Creature> parseFile()
//{
//    Creature creature;
//
//    std::string line = "";
//    std::ifstream file("Creatures.txt");
//
//    
//
//    if (!file.is_open())
//    {
//        std::cout << "file is not open";
//        return {};
//    }
//
//    std::vector<Creature> creatures{};
//
//    while (std::getline(file, line))
//    {
//        std::stringstream iss(line);
//        std::getline(iss, creature.name, ';');
//        std::getline(iss, creature.description, ';');
//        std::getline(iss, creature.tierTavern, ';');
//        std::getline(iss, creature.atk, ';');
//        std::getline(iss, creature.hp, ';');
//        std::getline(iss, creature.type, ';');
//
//        printCreature(creature);
//
//        creatures.push_back(creature);
//    }
//    return creatures;
//}

double calculateProbability(int value, int sumResult)
{
    return (double)value / (double)sumResult * 100.0;
}

void fillVectors(std::vector<Creature>& myCreatures, std::vector<Creature>& strangers, std::vector<Creature>& creatures)
{
    for (int i = 0, j = 7; i < 7 && j < creatures.size(); i++, j++)
    {
        myCreatures.push_back(creatures[i]);
        strangers.push_back(creatures[j]);
    }
}

void gameTable(std::vector<Creature> creatures)
{
    // Player creatures
    std::vector<Creature> myCreatures{};
    // Enemy creatures
    std::vector<Creature> strangers{};

    // Enemy damage
    int max = 0;
    int min = INT_MAX;

    // Player damage
    int myMax = 0;
    int myMin = INT_MAX;

    // Statistic
    int wins = 0;
    int loses = 0;
    int draws = 0;

    fillVectors(myCreatures, strangers, creatures);

    int count = 0;
    for (int x = 0; x < strangers.size(); x++)
    {
        for (int i = 0; count < myCreatures.size(); count++)
        {
            int j = x;
            i = count;
            while (!strangers.empty() && !myCreatures.empty())
            {
                // Check bubble
                if (myCreatures[i].bubble)
                    myCreatures[i].bubble = false;
                else
                    // Diminish HP
                    myCreatures[i].hp -= strangers[j].atk;

                if (strangers[j].bubble)
                    strangers[j].bubble = false;
                else
                    strangers[j].hp -= myCreatures[i].atk;

                // Delete creature
                if (myCreatures[i].hp < 1)
                    myCreatures.erase(myCreatures.begin() + i);

                if (strangers[j].hp < 1)
                {
                    strangers.erase(strangers.begin() + j);
                    j--;
                }

                j++;

                if (j >= strangers.size())
                    j = 0;

                if (i >= myCreatures.size())
                    i = myCreatures.size() - 1;
            }

            // Calculate statistic
            if (myCreatures.empty() && strangers.empty())
                draws++;
            else if (myCreatures.empty())
            {
                int sum = 0;

                loses++;
                
                for (auto creature : strangers)
                    sum += creature.tierTavern;
                sum += rand() % 6 + 1;

                if (sum > max)
                    max = sum;

                if (sum < min)
                    min = sum;
            }
            else
            {
                int sum = 0;

                wins++;

                for (auto creature : myCreatures)
                    sum += creature.tierTavern;
                sum += rand() % 6 + 1;

                if (sum > myMax)
                    myMax = sum;

                if (sum < myMin)
                    myMin = sum;
            }

            myCreatures.clear();
            strangers.clear();

            fillVectors(myCreatures, strangers, creatures);
        }

        count = 0;
    }

    // If 100% of value wins or losses remain big and wrong
    if (min == INT_MAX)
        min = 0;

    if (myMin == INT_MAX)
        myMin = 0;
    
    int sumResult = (wins + loses + draws);

    std::cout 
        << GREEN_TEXT << "Wins: " << calculateProbability(wins, sumResult) << std::endl
        << RED_TEXT << "Loses: " << calculateProbability(loses, sumResult) << std::endl
        << CYAN_TEXT << "Draws: " << calculateProbability(draws, sumResult) << std::endl
        << RESET;

    std::cout << "Player damage: " << myMin << "-" << myMax << std::endl;
    std::cout << "Enemy damage: " << min << "-" << max << std::endl;
}

int main()
{
    SetConsoleOutputCP(65001);
    srand(time(0));
    //removeFile();

    SQLiteDB db("Test.db");
    // Create a table in the database
    //db.executeQuery("CREATE TABLE IF NOT EXISTS creatures(" \
    //    "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
    //    "Tier_tavern    INT     NOT NULL," \
    //    "HP             INT     NOT NULL," \
    //    "ATK            INT     NOT NULL," \
    //    "Description    TEXT    NOT NULL," \
    //    "Name           TEXT    NOT NULL," \
    //    "Type           TEXT    NOT NULL," \
    //    "Kills          INT     NOT NULL," \
    //    "Deaths         INT     NOT NULL);");

    //std::vector<Creature> creatures = parseFile();

    //std::string sql = "INSERT INTO creatures (Tier_tavern, HP, ATK, Description, Name, Type, Kills, Deaths) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    // Fill the table
    //for (auto creature : creatures)
    //    db.executeQueryWithParams(sql, { creature.tierTavern, creature.hp, creature.atk, creature.description, creature.name, creature.type, "0", "0" });
    

    std::vector<Creature> creatures{};

    db.selectCreatures(&creatures);

    // Set the random characteristics
    for (int i = 0; i < creatures.size(); i++)
    {
        creatures[i].atk += rand() % 1000;
        creatures[i].hp += rand() % 1000;

        if (creatures[i].atk / creatures[i].hp > 50)
            creatures[i].hp += rand() % 50 + 50;

        creatures[i].bubble = (rand() % 1000) > 666;
    }

    std::cout << GREEN_TEXT << "===Player creatures===" << std::endl << RESET;
    for (int i = 0; i < creatures.size(); i++)
    {
        if(i == 7)
            std::cout << RED_TEXT << "===Enemy creatures===" << std::endl << RESET;
        printCreature(creatures[i]);
    }
    
    gameTable(creatures);
}
