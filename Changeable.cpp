#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <random>
#include <fstream>
using namespace std;
using namespace sf;

const string highScores("highscore.txt");
const int windowSize_x = 1000;
const int windowSize_y = 500;
const int groundOffset = windowSize_y - 150.f;
int gameSpeed = 8;
bool playerDead = false;
bool playDeadSound = false;
bool paused = false; // New variable to track pause state
Font pauseFont;       // Font for the pause text
Text pauseText;       // Text to display when paused

struct Fps_s
{
    Font font;
    Text text;
    Clock clock;
    int Frame;
    int fps;
};

class Fps
{
    Fps_s FPS;
public:
    Fps() : FPS() {
        if (FPS.font.loadFromFile("rsrc/Fonts/font.ttf")) {
            FPS.text.setFont(FPS.font);
        }
        FPS.text.setCharacterSize(15);
        FPS.text.setPosition(Vector2f(FPS.text.getCharacterSize() + 10.f, FPS.text.getCharacterSize()));
        FPS.text.setFillColor(Color(83, 83, 83));
    }
    void update()
    {
        if (FPS.clock.getElapsedTime().asSeconds() >= 0.5f) {
            FPS.fps = FPS.Frame;
            FPS.Frame = 0;
            FPS.clock.restart();
        }
        FPS.Frame++;
        FPS.text.setString("FPS :- " + to_string(FPS.fps));
    }
    void drawTo(RenderWindow& window)
    {
        window.draw(FPS.text);
    }

};


class SoundManager
{
public:
    SoundBuffer dieBuffer;
    SoundBuffer jumpBuffer;
    SoundBuffer pointBuffer;
    Sound dieSound;
    Sound jumpSound;
    Sound pointSound;
    Music backgroundMusic;

    SoundManager() : dieBuffer(), jumpBuffer(), pointBuffer(), dieSound(), jumpSound(), pointSound()
    {
        dieBuffer.loadFromFile("rsrc/Sounds/die.wav");
        jumpBuffer.loadFromFile("rsrc/Sounds/jump.wav");
        pointBuffer.loadFromFile("rsrc/Sounds/point.wav");
        backgroundMusic.openFromFile("rsrc/Sounds/background.ogg");

        dieSound.setBuffer(dieBuffer);
        jumpSound.setBuffer(jumpBuffer);
        pointSound.setBuffer(pointBuffer);
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(50.f);
        backgroundMusic.play();
    }

    void restartMusic()
    {
        backgroundMusic.stop();
        backgroundMusic.play();
    }
};


class Ground
{
public:
    Sprite groundSprite;
    Texture groundTexture;
    int offset = 0;
    Ground() : groundSprite(), groundTexture()
    {
        if (groundTexture.loadFromFile("rsrc/Images/GroundImage.png")) {
            groundSprite.setTexture(groundTexture);
            groundSprite.setPosition(Vector2f(0.f, windowSize_y - groundTexture.getSize().y - 50.f));
        }
    }

    void updateGround(bool isPaused)
    {
        if (isPaused) return;
        if (!playerDead) {
            if (offset > groundTexture.getSize().x - windowSize_x)
                offset = 0;

            offset += gameSpeed;
            groundSprite.setTextureRect(IntRect(offset, 0, windowSize_x, windowSize_y));
        }
        else {
            groundSprite.setTextureRect(IntRect(offset, 0, windowSize_x, windowSize_y));
        }
    }

    void reset()
    {
        offset = 0;
        groundSprite.setTextureRect(IntRect(0, 0, windowSize_x, windowSize_y));
    }

    void drawTo(RenderWindow& window)
    {
        window.draw(groundSprite);
    }

};

class Obstacle
{
public:
    Sprite obstacleSprite;
    FloatRect obstacleBounds{0.f, 0.f, 0.f, 0.f};

    bool isBird = false;
    vector <IntRect> birdFrames;
    Clock animationClock;
    int currentFrame = 0;
    RectangleShape Box;


    Obstacle(Texture& texture, bool bird = false) : obstacleSprite(), obstacleBounds(), isBird(bird){
        obstacleSprite.setTexture(texture);

        if (isBird){
            int frameWidth = 90;
            int frameHeight = 80;

            birdFrames.push_back(IntRect(0,0, frameWidth, frameHeight));
            birdFrames.push_back(IntRect(frameWidth,0, frameWidth, frameHeight));

            obstacleSprite.setTextureRect(birdFrames[0]);

            float randY = groundOffset - 100.f - rand() % 100;
            obstacleSprite.setPosition(Vector2f(windowSize_x, randY));
        }
        else{
            obstacleSprite.setPosition(Vector2f(windowSize_x, groundOffset));
        }
    }

    void update()
    {
        obstacleBounds = obstacleSprite.getGlobalBounds();
        obstacleBounds.width -= 15.f;
        obstacleBounds.height -= 15.f;

        obstacleSprite.move(-gameSpeed, 0.f);

        if (isBird && animationClock.getElapsedTime().asSeconds() > 0.2f) {
            currentFrame = (currentFrame + 1) % birdFrames.size();
            obstacleSprite.setTextureRect(birdFrames[currentFrame]);
            animationClock.restart();
        }

        updateBox();
    }

    void drawTo(RenderWindow& window)
    {
        window.draw(obstacleSprite);
    }

    void updateBox()
    {
        Box.setSize(Vector2f(obstacleBounds.width, obstacleBounds.height));
        Box.setFillColor(Color::Transparent);
        Box.setOutlineColor(Color(255, 0, 0));
        Box.setOutlineThickness(2.f);
        Box.setPosition(Vector2f(obstacleBounds.left, obstacleBounds.top));
    }
        
    void drawBox(RenderWindow& window)
    {
        window.draw(Box);
    }
};

class Obstacles
{
public:
    vector <Obstacle> obstacles;

    Time spawnTimer;
    Texture obstacleTexture_1;
    Texture obstacleTexture_2;
    Texture obstacleTexture_3;
    int randomNumber = 0;

    Obstacles() : spawnTimer(Time::Zero)
    {
        obstacles.reserve(3);

        obstacleTexture_1.loadFromFile("rsrc/Images/Cactus1.png");
        obstacleTexture_2.loadFromFile("rsrc/Images/Cactus2.png");
        obstacleTexture_3.loadFromFile("rsrc/Images/BirdSpriteSheet.png");
    }

    void update(Time& deltaTime, bool isPaused)
    {
        if (isPaused) return;
        spawnTimer += deltaTime;
        if (spawnTimer.asSeconds() > 0.5f + gameSpeed / 8){
            randomNumber = (rand() % 3) + 1;
            if (randomNumber == 1)
                obstacles.emplace_back(Obstacle(obstacleTexture_1));
            else if (randomNumber == 2)
                obstacles.emplace_back(Obstacle(obstacleTexture_2));
            else
                obstacles.emplace_back(Obstacle(obstacleTexture_3, true));
            spawnTimer = Time::Zero;
        }

        if (!playerDead) {
            for (int i = 0; i < obstacles.size(); i++) {
                obstacles[i].update();
                if (obstacles[i].obstacleSprite.getPosition().x < -150.f) {
                    obstacles.erase(obstacles.begin() + i);
                    i--;
                }
            }
        }
    }

    void drawTo(RenderWindow& window)
    {
        for (auto& obs : obstacles) {
            obs.drawTo(window);
//            obs.drawBox(window);
        }
    }

    void reset()
    {
        obstacles.clear();
    }
};
    
class Scores
{
public:
    Text previousScoreText;
    Text HIText;
    Text scoresText;
    Font scoresFont;
    SoundManager& soundM;
    short scores{ 0 };
    short previousScore{ 0 };
    short scoresIndex{ 0 };
    short scoresDiff{ 0 };
    short scoresInital;
    

    Scores(SoundManager& sm) : scoresFont(), scoresText(), previousScoreText(), scoresInital(), soundM(sm)
    {
        if (scoresFont.loadFromFile("rsrc/Fonts/Font.ttf")) {
            scoresText.setFont(scoresFont);
            scoresText.setCharacterSize(15);
            scoresText.setPosition(Vector2f(windowSize_x / 2 + windowSize_x / 4 + 185.f, scoresText.getCharacterSize() + 10.f));
            scoresText.setFillColor(Color(83, 83, 83));

            previousScoreText.setFont(scoresFont);
            previousScoreText.setCharacterSize(15);
            previousScoreText.setPosition(Vector2f(scoresText.getPosition().x - 100.f, scoresText.getPosition().y));
            previousScoreText.setFillColor(Color(83, 83, 83));

            HIText.setFont(scoresFont);
            HIText.setCharacterSize(15);
            HIText.setPosition(Vector2f(previousScoreText.getPosition().x - 50.f, previousScoreText.getPosition().y));
            HIText.setFillColor(Color(83, 83, 83));
        }
        HIText.setString("HI");
        scoresInital = 0;
    }

    void update()
    {
        if (!playerDead && !paused) { // Stop updating scores when paused
            scoresIndex++;
            if (scoresIndex >= 5) {
                scoresIndex = 0;
                scores++;
            }
            scoresDiff = scores - scoresInital;
            if (scoresDiff > 100) {
                scoresInital += 100;
                gameSpeed += 1;
                soundM.pointSound.play();
            }
            ifstream file(highScores); 
            int fileScore;
            if (file >> fileScore) {
                if (fileScore > previousScore) {
                    previousScore = fileScore;
                }
            }
            file.close();
            scoresText.setString(to_string(scores));
            previousScoreText.setString(to_string(previousScore));
        }
    }

    void reset()
{
    int allTimeHighScore;
    ifstream file(highScores);
    file >> allTimeHighScore;
    file.close();

    if (scores > previousScore){
        previousScore = scores;
        if (scores > allTimeHighScore)
        {
            allTimeHighScore = scores;
        }
    }

    ofstream outFile(highScores);
    outFile << allTimeHighScore;
    outFile.close();

    previousScoreText.setString(to_string(previousScore));
    scores = 0;  
}


    void drawTo(RenderWindow& window)
    {
        window.draw(scoresText);
        window.draw(previousScoreText);
        window.draw(HIText);
    }

};
class Dino
{
    public:
        Sprite dino;
        Vector2f dinoPos{ 0.f, 0.f };
        Vector2f dinoMotion{ 0.f, 0.f };
        Texture dinoTex;
        FloatRect dinoBounds;
        SoundManager& soundM;
        array <IntRect, 6> frames;
        Time timeTracker;
        int animationCounter{ 0 };
        RectangleShape Box;
        Text autoPlayText;
        Font autoPlayFont;

        Dino(SoundManager& sm) : dino(), dinoTex(), timeTracker(), soundM(sm)
        {
        float spriteWidth = 47;
        float spriteHeight = 56;
        if (dinoTex.loadFromFile("rsrc/Images/DinoWithDress.png")) {
            dino.setTexture(dinoTex);
            for (int i = 0; i < frames.size(); i++)
                frames[i] = IntRect(i * spriteWidth, 0, spriteWidth, spriteHeight);

            dino.setTextureRect(frames[0]);
            dinoPos = dino.getPosition();
        }
        else {
            cout << "Error loading the Dino Texture" << endl;
        }

        // Set font and text for auto-play
        if (autoPlayFont.loadFromFile("rsrc/Fonts/Font.ttf")) {
            autoPlayText.setFont(autoPlayFont);
            autoPlayText.setCharacterSize(20);
            autoPlayText.setFillColor(Color(83, 83, 83));
        }
    }

    void update(Time& deltaTime, vector<Obstacle>& obstacles, bool isPaused, Scores &scores)
    {
        if (isPaused) return;
        updateBox();
        dino.setScale(2.f,2.f);
        dinoPos = dino.getPosition();
        dinoBounds = dino.getGlobalBounds();
        dinoBounds.height -= 18.f;
        dinoBounds.width -= 10.f;
        dinoBounds.left += 4.f;

        timeTracker += deltaTime;

        for (auto& obs : obstacles)
            if (dinoBounds.intersects(obs.obstacleBounds)){
                playerDead = true;
                ifstream infile(highScores); 
                int fileScore;
                if (infile >> fileScore) {
                if (fileScore < scores.scores) {
                    infile.close();
                    ofstream outfile(highScores);
                    outfile << scores.scores;
                }
            }
            if (infile.is_open())
            {
                infile.close();
            }
            }

        if (!playerDead) {
            autoJump(obstacles);
            walk();
            if (Keyboard::isKeyPressed(Keyboard::Space) && dinoPos.y >= windowSize_y - 150.f) {
                animationCounter = 0;
                dinoMotion.y = -20.f;
                dino.setTextureRect(frames[2]);
                soundM.jumpSound.play();
            }

            if (dinoPos.y < windowSize_y - 290.f) {
                dinoMotion.y += 1.f;
                dino.setTextureRect(frames[3]);
            }

            if (dinoPos.y < windowSize_y - 150.f && dinoPos.y > windowSize_y - 290.f) {
                dinoMotion.y += 1.f;
                dino.setTextureRect(frames[2]);
            }


            if (dinoPos.y > windowSize_y - 150.f) {
                dino.setPosition(Vector2f(dino.getPosition().x, windowSize_y - 150.f));
                dino.setTextureRect(frames[1]);
                dinoMotion.y = 0.f;
            }

            dino.move(dinoMotion);
        }
        else {
            dinoMotion.y = 0.f;
            dino.setTextureRect(frames[4]);
            if (timeTracker.asSeconds() > 0.3f) {
                soundM.dieSound.stop();
                soundM.dieSound.setLoop(false);
                timeTracker = Time::Zero;
            }
            else {
                soundM.dieSound.play();
            }
        }
    }

    void walk()
    {
        if(animationCounter < 6)
            dino.setTextureRect(frames[0]);
        else
            dino.setTextureRect(frames[1]);
            
        animationCounter++;

        if (animationCounter >= 12)
            animationCounter = 0;

    }
    void reset()
    {
        dinoMotion.y = 0;
        dino.setPosition(Vector2f(dino.getPosition().x, windowSize_y - 150.f));
        dino.setTextureRect(frames[0]);
    }

    void drawTo(RenderWindow& window)
    {
        window.draw(dino);
        window.draw(autoPlayText);
    }


    void updateBox()
    {
        Box.setSize(Vector2f(dinoBounds.width, dinoBounds.height));
        Box.setFillColor(Color::Transparent);
        Box.setOutlineColor(Color(255, 0, 0));
        Box.setOutlineThickness(2.f);
        Box.setPosition(Vector2f(dinoBounds.left, dinoBounds.top));
    }
        
    void drawBox(RenderWindow& window)
    {
        window.draw(Box);
    }

    bool autoPlay = false;

    void toggleAutoPlay()
    {
        autoPlay = !autoPlay;
        if (autoPlay) {
            autoPlayText.setString("AutoPlay: ON");
            autoPlayText.setOrigin(autoPlayText.getLocalBounds().left + (autoPlayText.getLocalBounds().width / 2), autoPlayText.getLocalBounds().top + (autoPlayText.getLocalBounds().height / 2));
            autoPlayText.setPosition(Vector2f(windowSize_x / 2, 25.f));
        } else {
            autoPlayText.setString("");
        }
    }

    void autoJump(vector<Obstacle>& obstacles)
        {
            if (!autoPlay)
                return;

            for (auto& obs : obstacles)
            {
                float obsX = obs.obstacleSprite.getPosition().x;
                float obsY = obs.obstacleSprite.getPosition().y;
                float dinoX = dino.getPosition().x;

                if (obsX > dinoX && obsX - dinoX < 100.f + (gameSpeed * 10.f) && dino.getPosition().y >= windowSize_y - 150.f && obsY >= groundOffset)
                {
                    animationCounter = 0;
                    dinoMotion.y = -20.f;
                    dino.setTextureRect(frames[2]);
                    soundM.jumpSound.play();
                    break;
                }
            }
        }

};




class RestartButton
{
public:
    Sprite restartButtonSprite;
    FloatRect restartButtonSpriteBounds;
    Texture restartButtonTexture;
    Vector2f mousePos;
    bool checkPressed{ false };

    RestartButton() : restartButtonSprite(), restartButtonTexture(), mousePos(0.f, 0.f), restartButtonSpriteBounds()
    {
        if (restartButtonTexture.loadFromFile("rsrc/Images/RestartButton.png")) {
            restartButtonSprite.setTexture(restartButtonTexture);
            restartButtonSprite.setPosition(Vector2f(windowSize_x / 2 - restartButtonTexture.getSize().x / 2, windowSize_y / 2));
            restartButtonSpriteBounds = restartButtonSprite.getGlobalBounds();
        }
    }
};

/*
class Clouds
{
public:
    Sprite cloud;
    Texture cloudTexture;
    Time currTime;
    float speed;

    Clouds()
    {
        if (!cloudTexture.loadFromFile("rsrc/Images/Clouds.png")) {
            throw "Could not load cloud texture";
        }

        cloud.setTexture(cloudTexture);
        cloud.setPosition(Vector2f(windowSize_x, windowSize_y / 2.f - 40.f));
    }

    void updateClouds(Time& deltaTime, bool isPaused)
    {
        if (isPaused) return;
        currTime += deltaTime;

        if(!playerDead)
            speed = -1.f;
        else
            speed = -0.5f;

        cloud.move(Vector2f(speed, 0.f));

        if (cloud.getPosition().x < -cloudTexture.getSize().x) {
            int randomY = rand() % ((windowSize_y / 2 - 50) - (windowSize_y / 2 - 200) + 1) + (windowSize_y / 2 - 200);
            cloud.setPosition(Vector2f(windowSize_x, randomY));
        }
    }

    void drawTo(RenderWindow& window)
    {
        window.draw(cloud);
    }
};
*/

class DayNightCycle
{
public:
    Sprite sunSprite;
    Sprite moonSprite;
    Texture sunTexture;
    Texture moonTexture;
    CircleShape centerMarker;

    Clock cycleClock;
    Time pausedTime;
    float cycleDuration;
    vector<Color> gradientColors;

    DayNightCycle() : pausedTime(Time::Zero), cycleDuration(30.f)
    {
        //For center visualization
        float markerRadius = 5.f;
        centerMarker.setRadius(markerRadius);
        centerMarker.setFillColor(Color::Red);
        centerMarker.setOrigin(markerRadius, markerRadius); // center the circle
        centerMarker.setPosition(windowSize_x / 2, windowSize_y + 100.f);

        // Load textures
        if (!sunTexture.loadFromFile("rsrc/Images/Sun.png"))
            throw runtime_error("Could not load sun texture!");
        if (!moonTexture.loadFromFile("rsrc/Images/Moon.png"))
            throw runtime_error("Could not load moon texture!");

        // Assign textures to sprites
        sunSprite.setTexture(sunTexture);
        moonSprite.setTexture(moonTexture);

        // Set origins to center for circular movement
        sunSprite.setOrigin(sunTexture.getSize().x / 2.f, sunTexture.getSize().y / 2.f);
        moonSprite.setOrigin(moonTexture.getSize().x / 2.f, moonTexture.getSize().y / 2.f);

        // Optional scaling (adjust as needed)
        sunSprite.setScale(1.f, 1.f);
        moonSprite.setScale(1.f, 1.f);

        // Color gradient for background
        gradientColors = {
            Color(255, 223, 186), Color(255, 165, 0), Color(255, 165, 0),
            Color(255, 255, 224), Color(255, 255, 224), Color(255, 140, 0),
            Color(25, 25, 112),   Color(25, 25, 112),  Color(0, 0, 0),
            Color(0, 0, 0),       Color(25, 25, 112),  Color(25, 25, 112),
            Color(255, 223, 186)
        };
    }

    void update(bool isPaused)
    {
        if (!isPaused)
            pausedTime += cycleClock.restart();
        else
            cycleClock.restart();

        float elapsed = fmod(pausedTime.asSeconds(), cycleDuration);
        float angle = (elapsed / cycleDuration) * 360.f;

        float radiusX = 450.f;
        float radiusY = 400.f;
        float centerX = windowSize_x / 2.f;
        float centerY = windowSize_y + 100.f;
        float rad = angle * 3.14159f / 180.f;

        sunSprite.setPosition(centerX + cos(rad) * radiusX,
                              centerY - sin(rad) * radiusY);

        moonSprite.setPosition(centerX - cos(rad) * radiusX,
                               centerY + sin(rad) * radiusY);

        // Toggle visibility using alpha
        if (angle < 180.f) {
            sunSprite.setColor(Color(255, 255, 255, 255));
            moonSprite.setColor(Color(255, 255, 255, 0));
        } else {
            sunSprite.setColor(Color(255, 255, 255, 0));
            moonSprite.setColor(Color(255, 255, 255, 255));
        }
    }

    void reset()
    {
        pausedTime = Time::Zero;
        cycleClock.restart();
    }

    Color getBackgroundColor()
    {
        float elapsed = pausedTime.asSeconds();
        float t = fmod(elapsed / cycleDuration, 1.0f) * gradientColors.size();
        int index = static_cast<int>(t) % gradientColors.size();
        int nextIndex = (index + 1) % gradientColors.size();
        float blendFactor = t - index;

        return Color(
            gradientColors[index].r * (1 - blendFactor) + gradientColors[nextIndex].r * blendFactor,
            gradientColors[index].g * (1 - blendFactor) + gradientColors[nextIndex].g * blendFactor,
            gradientColors[index].b * (1 - blendFactor) + gradientColors[nextIndex].b * blendFactor
        );
    }

    void drawTo(RenderWindow& window)
    {
        window.draw(centerMarker);
        window.draw(sunSprite);
        window.draw(moonSprite);
    }
};



class GameState
{
public:
    Fps fps;
    Dino dino;
    Ground ground;
    Obstacles obstacles;
    Scores scores;
    //Clouds clouds;
    RestartButton restartButton;
    SoundManager soundManager;
    Font gameOverFont;
    Text gameOverText;
    Vector2f mousePos{ 0.f, 0.f };
    DayNightCycle dayNightCycle;

    GameState() : fps(), dino(soundManager), ground(), obstacles(), scores(soundManager), /*clouds(), */gameOverFont(), gameOverText(), soundManager(), dayNightCycle()
    {

        // Load font and set up game over text
        if(gameOverFont.loadFromFile("rsrc/Fonts/Font.ttf")){
            gameOverText.setFont(gameOverFont);
            dino.dino.setPosition(Vector2f(windowSize_x / 2 - windowSize_x / 4, windowSize_y - 150.f));
            gameOverText.setString("Game Over");
            gameOverText.setPosition(Vector2f(restartButton.restartButtonSprite.getPosition().x - 100.f,restartButton.restartButtonSprite.getPosition().y - 50));
            gameOverText.setFillColor(Color(83, 83, 83));
        }




    }
    void setMousePos(Vector2i p_mousePos)
    {
        mousePos.x = p_mousePos.x;
        mousePos.y = p_mousePos.y;
    }

    void update(Time deltaTime, bool isPaused)
    {
        restartButton.checkPressed = Mouse::isButtonPressed(Mouse::Left);
        if (playerDead && restartButton.restartButtonSpriteBounds.contains(mousePos) &&
            restartButton.checkPressed || (playerDead && Keyboard::isKeyPressed(Keyboard::R)))
        {
            ground.reset();
            obstacles.reset();
            dino.reset();
            soundManager.restartMusic();
            scores.reset();
            dayNightCycle.reset(); // Reset day-night cycle
            playerDead = false;
            gameSpeed = 8;
        }
        else if (!isPaused) // Only update game elements when not paused
        {
            ground.updateGround(isPaused);
            obstacles.update(deltaTime, isPaused);
            dino.update(deltaTime, obstacles.obstacles, isPaused,scores);
            //clouds.updateClouds(deltaTime, isPaused);
            scores.update();
            fps.update();
            dayNightCycle.update(isPaused); // Update day-night cycle
        }
        dayNightCycle.update(isPaused); // Pass the paused state to the day-night cycle
        fps.update();
    }

    void drawTo(RenderWindow& window)
    {
        window.clear(dayNightCycle.getBackgroundColor()); // Set gradient background
        dayNightCycle.drawTo(window); // Draw sun and moon (most behind layer)
        //clouds.drawTo(window); // Draw clouds
        ground.drawTo(window); // Draw ground
        obstacles.drawTo(window); // Draw obstacles
        dino.drawTo(window); // Draw dino
        scores.drawTo(window); // Draw scores
        fps.drawTo(window); // Draw FPS
        if (playerDead) {
            window.draw(gameOverText);
            window.draw(restartButton.restartButtonSprite);
        }
        if (paused) {
            window.draw(pauseText); // Draw pause text above all other layers
        }
    }

};

int main() {
    RenderWindow window(VideoMode(windowSize_x, windowSize_y), "Dino Game");
    window.setFramerateLimit(60);

    GameState game;
    Clock deltaClock;

    // Load font and set up pause text
    if (pauseFont.loadFromFile("rsrc/Fonts/Font.ttf")) {
        pauseText.setFont(pauseFont);
        pauseText.setString("Paused");
        pauseText.setCharacterSize(30);
        pauseText.setFillColor(Color(83, 83, 83));
        pauseText.setPosition(Vector2f(windowSize_x / 2 - pauseText.getGlobalBounds().width / 2, windowSize_y / 2 - 50));
    }

    while (window.isOpen()) {
        Time deltaTime = deltaClock.restart();
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close(); // Close the window
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                paused = !paused; // Toggle pause state
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::A && Keyboard::isKeyPressed(Keyboard::LControl)) 
                game.dino.toggleAutoPlay(); // Toggle auto-play state
        }

        game.setMousePos(Mouse::getPosition(window));
        game.update(deltaTime, paused);

        window.clear(Color::White);
        game.drawTo(window); // Draw everything, including pause text if paused


        // game.clouds.drawTo(window); // Draw clouds first
        // game.ground.drawTo(window); // Draw ground
        // game.obstacles.drawTo(window); // Draw obstacles
        // game.dino.drawTo(window); // Draw dino
        // game.scores.drawTo(window); // Draw scores
        // game.fps.drawTo(window); // Draw FPS
        // game.drawTo(window);
        // game.dino.drawBox(window);
        if (paused) {
            window.draw(pauseText); // Draw the pause text when paused
        }
        window.display();
    }

    return 0;
}