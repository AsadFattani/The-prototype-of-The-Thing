#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <random>
using namespace std;
using namespace sf;

const int windowSize_x = 1000;
const int windowSize_y = 500;
const int groundOffset = windowSize_y - 150.f;
int gameSpeed = 8;
bool playerDead = false;
bool playDeadSound = false;

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
        if (FPS.clock.getElapsedTime().asSeconds() >= 1.f) {
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
/*
class SoundManager
{
public:
    SoundBuffer dieBuffer;
    SoundBuffer jumpBuffer;
    SoundBuffer pointBuffer;
    Sound dieSound;
    Sound jumpSound;
    Sound pointSound;

    SoundManager() : dieBuffer(), jumpBuffer(), pointBuffer(), dieSound(), jumpSound(), pointSound()
    {
        dieBuffer.loadFromFile("rsrc/Sounds/die.wav");
        jumpBuffer.loadFromFile("rsrc/Sounds/jump.wav");
        pointBuffer.loadFromFile("rsrc/Sounds/point.wav");

        dieSound.setBuffer(dieBuffer);
        jumpSound.setBuffer(jumpBuffer);
        pointSound.setBuffer(pointBuffer);
    }
};
*/

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

    void updateGround()
    {
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

};

class Obstacle
{
public:
    Sprite obstacleSprite;
    FloatRect obstacleBounds{0.f, 0.f, 0.f, 0.f};

    Obstacle(Texture& texture) : obstacleSprite(), obstacleBounds() {
        obstacleSprite.setTexture(texture);
        obstacleSprite.setPosition(Vector2f(windowSize_x, groundOffset));
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
        obstacleTexture_3.loadFromFile("rsrc/Images/Cactus3.png");
    }

    void update(Time& deltaTime)
    {
        spawnTimer += deltaTime;
        if (spawnTimer.asSeconds() > 0.5f + gameSpeed / 8){
            randomNumber = (rand() % 3) + 1;
            if (randomNumber == 1)
                obstacles.emplace_back(Obstacle(obstacleTexture_1));
            else if (randomNumber == 2)
                obstacles.emplace_back(Obstacle(obstacleTexture_2));
            else
                obstacles.emplace_back(Obstacle(obstacleTexture_3));

            spawnTimer = Time::Zero;
        }

        if (!playerDead) {
            for (int i = 0; i < obstacles.size(); i++) {
                obstacles[i].obstacleBounds = obstacles[i].obstacleSprite.getGlobalBounds();
                obstacles[i].obstacleBounds.width -= 10.f;
                obstacles[i].obstacleSprite.move(-gameSpeed, 0.f);
                if (obstacles[i].obstacleSprite.getPosition().x < -150.f) {
                    obstacles.erase(obstacles.begin() + i);
                    i--;
                }
            }
        }
    }

    void drawTo(RenderWindow& window)
    {
        for (auto& obstacles : obstacles) {
            window.draw(obstacles.obstacleSprite);
        }
    }

    void reset()
    {
        obstacles.clear();
    }
};
/*
class Dino
{
public:
    Sprite dino;
    Vector2f dinoPos{ 0.f, 0.f };
    Vector2f dinoMotion{ 0.f, 0.f };
    Texture dinoTex;
    FloatRect dinoBounds;
    SoundManager soundManager;
    array<IntRect, 6> frames;
    Time timeTracker;
    int animationCounter{ 0 };

    Dino() : dino(), dinoTex(), soundManager(), timeTracker()
    {
        if (dinoTex.loadFromFile("rsrc/Images/PlayerSpriteSheet.png")) {
            dino.setTexture(dinoTex);
            for (int i = 0; i < frames.size(); i++)
                frames[i] = IntRect(i * 90, 0, 90, 95);
            dino.setTextureRect(frames[0]);
            dinoPos = dino.getPosition();
        }
        else {
            cout << "Error loading the PlayerSprite texture" << endl;
        }
    }

    void update(Time& deltaTime, vector<Obstacle>& obstacles)
    {
        dinoPos = dino.getPosition();
        dinoBounds = dino.getGlobalBounds();
        dinoBounds.height -= 15.f;
        dinoBounds.width -= 10.f;
        timeTracker += deltaTime;

        for (auto& obs : obstacles)
            if (dinoBounds.intersects(obs.obstacleBounds))
                playerDead = true;

        if (!playerDead) {
            walk();
            if (Keyboard::isKeyPressed(Keyboard::Space) && dinoPos.y >= windowSize_y - 150.f) {
                animationCounter = 0;
                dinoMotion.y = -20.f;
                dino.setTextureRect(frames[1]);
                soundManager.jumpSound.play();
            }

            if (dinoPos.y < windowSize_y - 150.f) {
                dinoMotion.y += 1.f;
                dino.setTextureRect(frames[1]);
            }

            if (dinoPos.y > windowSize_y - 150.f) {
                dino.setPosition(Vector2f(dino.getPosition().x, windowSize_y - 150.f));
                dinoMotion.y = 0.f;
            }

            dino.move(dinoMotion);
        }
        else {
            dinoMotion.y = 0.f;
            dino.setTextureRect(frames[3]);
            if (timeTracker.asMilliseconds() > 170.f) {
                soundManager.dieSound.stop();
                soundManager.dieSound.setLoop(false);
                timeTracker = Time::Zero;
            }
            else {
                soundManager.dieSound.play();
            }
        }
    }

    void walk()
    {
        for (int i = 0; i < frames.size() - 3; i++)
            if (animationCounter == i * 3)
                dino.setTextureRect(frames[i]);

        if (animationCounter >= (frames.size() - 2) * 3)
            animationCounter = 0;

        animationCounter++;
    }
    void reset()
    {
        dinoMotion.y = 0;
        dino.setPosition(Vector2f(dino.getPosition().x, windowSize_y - 150.f));
        dino.setTextureRect(frames[0]);
    }

};
*/
class Scores
{
public:
    Text previousScoreText;
    Text HIText;
    Text scoresText;
    Font scoresFont;
//    SoundManager soundManager;
    short scores{ 0 };
    short previousScore{ 0 };
    short scoresIndex{ 0 };
    short scoresDiff{ 0 };
    short scoresInital;

    Scores() : scoresFont(), scoresText(), previousScoreText(), scoresInital() /* soundManager() */
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
        if (!playerDead) {
            scoresIndex++;
            if (scoresIndex >= 5) {
                scoresIndex = 0;
                scores++;
            }
            scoresDiff = scores - scoresInital;
            if (scoresDiff > 100) {
                scoresInital += 100;
                gameSpeed += 1;
                //soundManager.pointSound.play();
            }

            scoresText.setString(to_string(scores));
            previousScoreText.setString(to_string(previousScore));
        }
    }

    void reset()
    {
        if (scores > previousScore)
            previousScore = scores;

        previousScoreText.setString(to_string(previousScore));
        scores = 0;
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

class Clouds
{
public:
    vector<Sprite> clouds;
    Time currTime;
    Texture cloudTexture;

    Clouds() : cloudTexture(), clouds(), currTime()
    {
        cloudTexture.loadFromFile("rsrc/Images/Clouds.png");
        clouds.reserve(4);
        clouds.emplace_back(Sprite(cloudTexture));
        clouds.back().setPosition(Vector2f(windowSize_x, windowSize_y / 2 - 40.f));
    }

    void updateClouds(Time& deltaTime)
    {
        currTime += deltaTime;
        if (currTime.asSeconds() > 8.f) {
            clouds.emplace_back(Sprite(cloudTexture));
            int randomY = rand() % ((windowSize_y / 2 - 50) - (windowSize_y / 2 - 200) + 1) + (windowSize_y / 2 - 200);
            clouds.back().setPosition(Vector2f(windowSize_x, randomY));
            currTime = Time::Zero;
        }

        for (int i = 0; i < clouds.size(); i++) {
            if (!playerDead)
                clouds[i].move(Vector2f(-1.f, 0.f));
            else
                clouds[i].move(Vector2f(-0.5f, 0.f));

            if (clouds[i].getPosition().x < 0.f - cloudTexture.getSize().x) {
                clouds.erase(clouds.begin() + i);
                --i;
            }
        }
    }

    void drawTo(RenderWindow& window)
    {
        for (auto& clouds : clouds) {
            window.draw(clouds);
        }
    }

};

class GameState
{
public:
    Fps fps;
//    Dino dino;
    Ground ground;
    Obstacles obstacles;
    Scores scores;
    Clouds clouds;
    RestartButton restartButton;
    Font gameOverFont;
    Text gameOverText;
    Vector2f mousePos{ 0.f, 0.f };

    GameState() : fps(), /*dino(),*/ ground(), obstacles(), scores(), clouds(), gameOverFont(), gameOverText()
    {
        gameOverFont.loadFromFile("rsrc/Fonts/Font.ttf");
        gameOverText.setFont(gameOverFont);
//        dino.dino.setPosition(Vector2f(windowSize_x / 2 - windowSize_x / 4, windowSize_y - 150.f));
        gameOverText.setString("Game Over");
        gameOverText.setPosition(Vector2f(restartButton.restartButtonSprite.getPosition().x - gameOverText.getCharacterSize(),
            restartButton.restartButtonSprite.getPosition().y - 50));
        gameOverText.setFillColor(Color(83, 83, 83));
    }
    void setMousePos(Vector2i p_mousePos)
    {
        mousePos.x = p_mousePos.x;
        mousePos.y = p_mousePos.y;
    }

    void update(Time deltaTime)
    {
        restartButton.checkPressed = Mouse::isButtonPressed(Mouse::Left);
        if (playerDead && restartButton.restartButtonSpriteBounds.contains(mousePos) &&
            restartButton.checkPressed)
        {
            ground.reset();
            obstacles.reset();
//            dino.reset();
            scores.reset();
            playerDead = false;
            gameSpeed = 8;
        }
        else
        {
            ground.updateGround();
            obstacles.update(deltaTime);
//            dino.update(deltaTime, obstacles.obstacles);
            clouds.updateClouds(deltaTime);
            scores.update();
        }
        fps.update();
    }

    void drawTo(RenderWindow& window)
    {
        clouds.drawTo(window);
        window.draw(ground.groundSprite);
        obstacles.drawTo(window);
        window.draw(scores.scoresText);
        window.draw(scores.previousScoreText);
        window.draw(scores.HIText);
//        window.draw(dino.dino);

        if (playerDead)
        {
            window.draw(gameOverText);
            window.draw(restartButton.restartButtonSprite);
        }

        fps.drawTo(window);
    }

};

int main() {
    RenderWindow window(VideoMode(windowSize_x, windowSize_y), "Dino Game");
    window.setFramerateLimit(60);

    GameState game;
    Clock deltaClock;

    while (window.isOpen()) {
        Time deltaTime = deltaClock.restart();
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        game.setMousePos(Mouse::getPosition(window));
        game.update(deltaTime);

        window.clear(Color::White);
        game.clouds.drawTo(window);
        
        window.draw(game.ground.groundSprite);
        game.obstacles.drawTo(window);
//        window.draw(game.dino.dino);
        game.scores.update();
        window.draw(game.scores.scoresText);
        window.draw(game.scores.previousScoreText);
        window.draw(game.scores.HIText);

        if (playerDead) {
            window.draw(game.restartButton.restartButtonSprite);
            window.draw(game.gameOverText);
        }

        game.fps.drawTo(window);
        window.display();
    }

    return 0;
}

