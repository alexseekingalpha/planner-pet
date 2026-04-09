#include <Arduino.h>


const char* STAGES[] = {"Egg", "Baby", "Kid", "Teen", "Adult", "Elder"};
const int XP_THRESHOLDS[] = {0, 10, 30, 60, 100, 150};
const int NUM_STAGES = 6;

const char* STAGE_ART[] = {
    "  (.)  ",    // Egg
    " (^_^) ",    // Baby
    " (^o^) ",    // Kid
    "<(^_^)>",    // Teen
    "<(^O^)>",    // Adult
    "*<(^O^)>*"   // Elder
};

// Pet 
struct Pet {
    int seed;
    String name;
    int bodyShape;    
    int colourIndex;  
    int personality;  
    int xp;
    int coins;
    int missStreak;
};

// Tasks
struct Task {
    String name;
    int duration;     
    bool completed;
};

//  Globals
Pet pet;
Task tasks[10];       
int taskCount = 0;

//Helpers

String readLine() {
    while (!Serial.available()) {
        delay(10);
    }
    String input = Serial.readStringUntil('\n');
    input.trim();
    return input;
}


int readInt() {
    String line = readLine();
    return line.toInt();
}


void generateTraits() {
    pet.bodyShape   = pet.seed % 3;
    pet.colourIndex = pet.seed % 8;
    pet.personality = (pet.seed / 1000) % 3;
}


int getStage(int xp) {
    for (int i = NUM_STAGES - 1; i >= 0; i--) {
        if (xp >= XP_THRESHOLDS[i]) return i;
    }
    return 0;
}


void showPet() {
    int stage = getStage(pet.xp);
    const char* bodies[] = {"Round", "Tall", "Chunky"};
    const char* personalities[] = {"Calm", "Hyper", "Sleepy"};

    Serial.println();
    Serial.println("====== " + pet.name + " ======");
    Serial.print("     ");
    Serial.println(STAGE_ART[stage]);
    Serial.println("  Stage: " + String(STAGES[stage]));
    Serial.println("  XP: " + String(pet.xp) + "  Coins: " + String(pet.coins));
    Serial.println("  Body: " + String(bodies[pet.bodyShape]) +
                   "  Personality: " + String(personalities[pet.personality]));
    if (pet.missStreak > 0) {
        Serial.println("  Miss streak: " + String(pet.missStreak) + " (pet is sad!)");
    }
    Serial.println("========================");
}


void addTask() {
    if (taskCount >= 10) {
        Serial.println("Task list full! End the day first.");
        return;
    }

    Serial.print("Task name: ");
    tasks[taskCount].name = readLine();

    Serial.print("Duration (minutes): ");
    tasks[taskCount].duration = readInt();
    tasks[taskCount].completed = false;
    taskCount++;

    Serial.println("Task added!");
}

void doTasks() {
    if (taskCount == 0) {
        Serial.println("No tasks! Add some first.");
        return;
    }

    Serial.println("\n--- Today's Tasks ---");
    for (int i = 0; i < taskCount; i++) {
        Serial.print(String(i + 1) + ". [");
        Serial.print(tasks[i].completed ? "X" : " ");
        Serial.println("] " + tasks[i].name + " (" + String(tasks[i].duration) + " min)");
    }

    Serial.print("Complete which task? (0 to cancel): ");
    int choice = readInt();

    if (choice < 1 || choice > taskCount) return;

    Task &t = tasks[choice - 1];
    if (t.completed) {
        Serial.println("Already done!");
        return;
    }

    t.completed = true;
    int xpGain = 5 + t.duration / 10;
    int coinGain = 2 + t.duration / 15;

    int oldStage = getStage(pet.xp);
    pet.xp += xpGain;
    pet.coins += coinGain;
    pet.missStreak = 0;
    int newStage = getStage(pet.xp);

    Serial.println("Done! +" + String(xpGain) + " XP, +" + String(coinGain) + " coins");
    if (newStage > oldStage) {
        Serial.println("*** " + pet.name + " grew into " + String(STAGES[newStage]) + "! ***");
    }
}

void endOfDay() {
    int missed = 0;
    int penalty = 0;

    for (int i = 0; i < taskCount; i++) {
        if (!tasks[i].completed) {
            missed++;
            penalty += 100 / max(tasks[i].duration, 1);
        }
    }

    if (missed == 0) {
        Serial.println("\nAll tasks done today! " + pet.name + " is happy!");
    } else {
        pet.missStreak += missed;
        pet.xp = max(0, pet.xp - penalty);
        Serial.println("\nMissed " + String(missed) + " task(s). -" +
                       String(penalty) + " XP. " + pet.name + " is sad...");
    }

    // Clear tasks for next day
    taskCount = 0;
}

//  Menu 

void showMenu() {
    Serial.println("\n1. Add task");
    Serial.println("2. Do tasks");
    Serial.println("3. End day");
    Serial.print("> ");
}

// 

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("=== PLANNER PET ===");
    Serial.print("Name your pet: ");
    pet.name = readLine();

    
    randomSeed(analogRead(0) ^ millis());
    pet.seed = random(0, 100000);
    pet.xp = 0;
    pet.coins = 0;
    pet.missStreak = 0;
    generateTraits();

    Serial.println(pet.name + " hatched! (Seed: " + String(pet.seed) + ")");
}

void loop() {
    showPet();
    showMenu();

    int choice = readInt();

    switch (choice) {
        case 1: addTask(); break;
        case 2: doTasks(); break;
        case 3: endOfDay(); break;
        default: Serial.println("Pick 1-3."); break;
    }
}
