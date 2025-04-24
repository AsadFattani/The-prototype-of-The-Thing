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

class tempBox
{
    RectangleShape Box;
    public:
        tempBox() : Box()
        {
            Box.setSize(Vector2f(90.f, 100.f));
            Box.setFillColor(Color::Transparent);
            Box.setOutlineColor(Color(83, 83, 83));
            Box.setOutlineThickness(5.f);
            Box.setPosition(Vector2f(200.f, groundOffset));
        }
        void drawTo(RenderWindow& window)
        {
            window.draw(Box);
        }
};

class Scores
{
public:
    Text previousScoreText;
    Text HIText;
    Text scoresText;
    Font scoresFont;
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
    Ground ground;
    Obstacles obstacles;
    Scores scores;
    Clouds clouds;
    RestartButton restartButton;
    tempBox Box;
    Font gameOverFont;
    Text gameOverText;
    Vector2f mousePos{ 0.f, 0.f };

    GameState() : ground(), obstacles(), scores(), clouds(), gameOverFont(), gameOverText()
    {
        gameOverFont.loadFromFile("rsrc/Fonts/Font.ttf");
        gameOverText.setFont(gameOverFont);
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
            restartButton.checkPressed || (playerDead && Keyboard::isKeyPressed(Keyboard::R)))
        {
            ground.reset();
            obstacles.reset();
            scores.reset();
            playerDead = false;
            gameSpeed = 8;
        }
        else
        {
            ground.updateGround();
            obstacles.update(deltaTime);
            clouds.updateClouds(deltaTime);
            scores.update();
        }
    }

    void drawTo(RenderWindow& window)
    {
        clouds.drawTo(window);
        window.draw(ground.groundSprite);
        obstacles.drawTo(window);
        window.draw(scores.scoresText);
        window.draw(scores.previousScoreText);
        window.draw(scores.HIText);

        if (playerDead)
        {
            window.draw(gameOverText);
            window.draw(restartButton.restartButtonSprite);
        }

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
        game.Box.drawTo(window);
        game.scores.update();
        window.draw(game.scores.scoresText);
        window.draw(game.scores.previousScoreText);
        window.draw(game.scores.HIText);

        if (playerDead) {
            window.draw(game.restartButton.restartButtonSprite);
            window.draw(game.gameOverText);
        }

        window.display();
    }

    return 0;
}

