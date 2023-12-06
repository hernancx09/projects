# Space Invaders
I found a tutorial on youtube to create the basic game you see here. I am currently trying to implement AI into the aliens so the game becomes harder as you play. 

Original Idea: Create a version of the space invadors retro game that analyzes your moves and uses AI to create harder levels as you progress. Im thinking we would code the visuals, maybe start with basic letters and move into actual graphics later. 

What actually ended up happening:
I coded a version of it in C that used V's for the alien ships, U's for their shots, i's for my shots, and a A for my ship. There were multiple levels of dificulty but the aliens stayed at the top and didnt come down. I was going to start implementing it in python using Pygame when I realized it had been done before, so I followed a tutorial to create it and this completely changed my first Idea. 
I was able to start coding the game on my computer, but i wasn't able to run it. The main issue is that my system didnt have the librarys necessary to actually test and run the game. So I began downloading them but I ran into some problems with installing them,but in the end it started working and I got to testing. Oonce I had finished coding the background and game physics like bullet and spaceship collisions, I realized it was going take a lot more time than I had in the semester to code the move analysis so I started working with implimenting a speed shot ability without looking it up so I could challenge myself to do it from scratch. This took the most time since I tried raising and lowering the bullet delay(which did absolutely nothing), I tried making things global variables which was also to no avail. I had to restructure the spaceship class to be able to work with the fluxuating bullet delay I was trying to incorporate. From there I could seemlesly change it in the main file and pass it into the class and it started to work. This ability makes shooting speed go 70% faster for 5 seconds then return to normal. It is activated by shooting down one of the red 'Boss' alien ships that pass through the top of the screen from time to time.

I also added 2 'easter egg' cheats to the game which make you shoot a laser insted of a bullet every second (which makes the game really cool but also destroys everything in its path so it makes it super easy too) and an 'infinite' lives cheat which just gives you 99 lives, but for this game that should be enough to be called infinite since theres no way you get shot 99 times before the aliens get to you.

I was going to incorporate a level select, but I spent way too much time on implementing the ability and cheats perfectly so there wouldnt be any risk of crashing the game at any point so I scrapped the idea. I removed my makeshift settings button, and level select buttons and started to clean up the visuals so I would have a decent nice game to turn in on time. After turning in this project I do think I will keep working on it to incorporate everything I wanted to.

Now to the point:
How to play this game

run main.py, it will open the game to a main menu with the name and a play button. From there its pretty straight forward.

a or left_arrow to move left.
d or right_arrow to move right.
space or up_arrow to shoot.

shooting the red 'boss' alien that appears grants a speed shot ability for 5 seconds

Cheats:
pressing p will grant user a laser shot (cool cool cool)
pressing l will grant user 99 lives (who needs this many?!?)

at the end, highscore is calculated and it boots the player back to main menu.
to quit press esc or x out of the window.