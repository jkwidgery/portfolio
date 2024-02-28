#Coding Assignment 1
#MAT340 Spring 2024 Vermessi
#Jasmine Widgery

#IMPORTS
import random as rand
#END IMPORTS

prompt = "Which test would you like to run? \n1. Coupon Collector \n2. Monty Hall Applet \n3. Monty Hall Simulation \n4. Gamblers Ruin\n"

global testNumber
try:
    testNumber = int(input(prompt))
except ValueError:
       raise Exception("That\'s not a number!")
else:
    if testNumber not in [1, 2, 3, 4]:
        raise Exception("Please enter a value between 1 and 4.")


if testNumber == 1:
##############TEST 1 - COUPON COLLECTOR##############
    NUM_ITERATIONS = 100
    numTypes = 0
    print("You chose " + str(testNumber) + ", Coupon Collector.")
    try:
        numTypes = int(input("Please enter the number of coupon types:"))
    except ValueError:
        raise Exception("That\'s not a number!")
        
    avgCollected = 0
    for i in range(0, NUM_ITERATIONS): #run at least 100 trials
        totalCollected = 0
        typesDrawn = set() # create set of unique types 
        while len(typesDrawn) < numTypes:
            #draw coupon
            typeDrawn = rand.randrange(0, numTypes)
            totalCollected += 1
            
            #add new coupon to set if it's unique
            typesDrawn.add(typeDrawn) 
        #print(str(totalCollected) + " coupons selected until all types were chosen.")    
        avgCollected += totalCollected
    avgCollected = avgCollected / NUM_ITERATIONS
    print("The average number of coupons collected to complete set is : " + str(avgCollected))
        
##############END TEST 1 - COUPON COLLECTOR##############      

elif testNumber == 2:
##############TEST 2 - MONTY HALL APPLET##############
    print("You chose " + str(testNumber) + ", Monty Hall Applet.")
    excludedDoors = set()
    NUM_DOORS = 3
    playerChoice = 0
    carDoor = rand.randrange(1, 4)
    excludedDoors.add(carDoor)
    try:
        playerChoice = int(input("Pick a door 1, 2, or 3:"))
    except ValueError:
        raise Exception("That\'s not a number!")
    else:
        if playerChoice not in [1, 2, 3]:
            raise Exception("Please enter a value between 1 and 3.")
        
    excludedDoors.add(playerChoice)
    opened = rand.choice([i for i in range(1,4) if i not in excludedDoors])
    try:
        staySwitch = str(input("You chose door " + str(playerChoice) + ". I opened door " + str(opened) + ". \nEnter 'stay' if you want to maintain your choice, or 'switch' if you want to choose the last door.\n"))
        staySwitch = staySwitch.upper().strip()
    except ValueError:
        print("That\'s not a string!")
    else:
        if (staySwitch != "SWITCH") and (staySwitch != "STAY"):
            print("You entered " + staySwitch)
            raise Exception("Please enter either 'stay' or 'switch'") 
            
    if (playerChoice == carDoor and staySwitch == "STAY") or (playerChoice != carDoor and staySwitch == "SWITCH"):
        print("Congrats! The door was behind door " + str(carDoor))
    else:
        print("Oops! No car for you. It was behind door " + str(carDoor))
            
##############END TEST 2 - MONTY HALL APPLET##############           
elif testNumber == 3:
##############TEST 3 - MONTY HALL SIM##############
    print("You chose " + str(testNumber) + ", Monty Hall Simulation.") 
    NUM_ITERATIONS = 100
    numDoors = 0
    try:
        numDoors = int(input("Input the total number of doors: "))
    except ValueError:
        raise Exception("That\'s not a number!")
        
    numCars = 0
    try:
        numCars = int(input("Input the total number of cars: "))
    except ValueError:
        raise Exception("That\'s not a number!")
        
    numToOpen = 0
    try:
        numToOpen = int(input("Input the number of non-car doors you wish to open: "))
    except ValueError:
        raise Exception("That\'s not a number!")
    
    playerDoor = 1
    allDoors = set(range(1, numDoors + 1))
    numWins = 0
    for i in range(0, NUM_ITERATIONS):
        carDoors = set(rand.sample(sorted(allDoors), numCars))
            
        excludedSet = allDoors - carDoors - {playerDoor} #doors we CAN open
        #print(excludedSet)
        opened = set(rand.sample(sorted(excludedSet), numToOpen))
        
        unopenedDoors = allDoors - opened
        possibleNewDoors = unopenedDoors - {playerDoor}
        newPlayerDoor = rand.choice(list(possibleNewDoors))
        #DEBUG
        #print("Car door: " + str(carDoors))
        #print("Opened: " + str(opened))
        #print("Unopened: " + str(unopenedDoors))
        #print("Player choice " + str(newPlayerDoor))
        if newPlayerDoor in carDoors:
            numWins += 1
    print("Estimated probability with given values is: " + str(numWins / NUM_ITERATIONS))
        
            
##############END TEST 3 - MONTY HALL SIM##############  
elif testNumber == 4:
##############TEST 4 - GAMBLER'S RUIN##############   
    print("You chose " + str(testNumber) + ", Gambler\'s Ruin.")
    NUM_ITERATIONS = 100
    startAmt = 0
    try:
        startAmt = int(input("Input the starting dollar amount: "))
    except ValueError:
        raise Exception("That\'s not a number!")
        
    winAmt = 0
    try:
        winAmt = int(input("Input the winning dollar amount you wish to stop at: "))
    except ValueError:
        raise Exception("That\'s not a number!")
    else:
        if winAmt <= startAmt:
            raise Exception("Oops, the winning amount must be more than the starting amount.")
        
    probWin = 0
    try:
        probWin = float(input("Input the probability of winning a match:"))
    except ValueError:
        raise Exception("That\'s not a number!")
    intProbWin = int(probWin * 10)
    wins = 0
    losses = 0
    for i in range(0, NUM_ITERATIONS):
        currAmt = startAmt
        while(True):
            chance = rand.randrange(1, 11)
            #print(chance)
            if chance <= intProbWin:
                currAmt += 1
            else:
                currAmt -= 1
                
            if currAmt >= winAmt:
                wins += 1
                break
            if currAmt <= 0:
                losses += 1
                break
    print("Num wins given starting amount " + str(startAmt) + "$, winning amount " + str(winAmt) + "$, and probability " + str(probWin) + " is " + str(wins)) 
    
    
exit = input("If you'd like to play another game, you must exit and relaunch the program.\nPress 'Enter' to exit.") 
    