#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>  // 為了使用 rand()
#include <ctime>    // 為了使用 time()
#include <iostream>  // 添加這行
#include <filesystem>  // 添加這行
#include <memory>  // 添加這行
using namespace sf;
using namespace std;

// 在檔案開頭定義全域常量
const float BOUNDARY_LEFT = 200.f;    // 左邊界
const float PLAY_AREA_WIDTH = 800.f;  // 遊戲區域寬度
const float ENEMY_WIDTH = 30.f;       // 敵人寬度
const float BOUNDARY_RIGHT = BOUNDARY_LEFT + PLAY_AREA_WIDTH;  // 右邊界

class Bullet {
public:
    CircleShape shape;
    float speed;

    Bullet(float startX, float startY) {
        speed = 1.0f;
        shape.setRadius(5.f);
        shape.setFillColor(Color::Yellow);
        shape.setPosition(startX, startY);
    }

    void update() {
        shape.move(0, -speed);
    }
};

// 添加敵人類
class Enemy {
public:
    sf::RectangleShape shape;
    float speed;
    
    Enemy(float startX, float startY) {
        speed = 0.1f;
        shape.setSize(sf::Vector2f(30.f, 30.f));  // 確保敵人有合適的大小
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Red);
    }

    void update() {
        shape.move(0, speed);
    }

    // 修改碰撞檢測函數以使用 Sprite
    bool checkCollision(const Sprite& player) const {
        // 獲取精靈的邊界框
        FloatRect playerBounds = player.getGlobalBounds();
        FloatRect enemyBounds = shape.getGlobalBounds();
        
        return enemyBounds.intersects(playerBounds);
    }
};

class AnimatedBackground {
private:
    std::vector<sf::Texture> frames;
    sf::Sprite sprite;
    float frameTime;
    float currentTime;
    size_t currentFrame;
    sf::Vector2f scale;

public:
    AnimatedBackground(const std::vector<std::string>& framePaths, float frameDuration, const sf::Vector2f& windowSize) {
        frameTime = frameDuration;
        currentTime = 0.0f;
        currentFrame = 0;

        // 加載所有幀
        for (const auto& path : framePaths) {
            sf::Texture texture;
            if (!texture.loadFromFile(path)) {
                std::cout << "Error loading frame: " << path << std::endl;
                continue;
            }
            frames.push_back(texture);
        }

        if (!frames.empty()) {
            sprite.setTexture(frames[0]);
            
            // 計算縮放比例以適口
            float scaleX = windowSize.x / frames[0].getSize().x;
            float scaleY = windowSize.y / frames[0].getSize().y;
            scale = sf::Vector2f(scaleX, scaleY);
            sprite.setScale(scale);
        }
    }

    void update(float deltaTime) {
        if (frames.empty()) return;

        currentTime += deltaTime;
        if (currentTime >= frameTime) {
            currentTime = 0;
            currentFrame = (currentFrame + 1) % frames.size();
            sprite.setTexture(frames[currentFrame]);
            sprite.setScale(scale);  // 確保縮放保持不變
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

class Game {
private:
    RenderWindow& window;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int* killCountPtr;
    int* goldPtr;  // 添加金幣指針
    std::unique_ptr<AnimatedBackground> background;  // 使用智能指針管理背景

public:
    Game(RenderWindow& win, int* killCount, int* gold) 
        : window(win), killCountPtr(killCount), goldPtr(gold) {
        // 輸出當前工作目錄
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        
        std::vector<std::string> framePaths;
        for (int i = 1; i <= 24; i++) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "/Users/cpcap/GTA6/texture/background/frames/frame_%03d.png", i);
            std::string path = buffer;
            std::cout << "Trying to load: " << path << std::endl;  // 輸出嘗試加載的路徑
            framePaths.push_back(path);
        }

        background = std::make_unique<AnimatedBackground>(
            framePaths, 
            0.1f, 
            sf::Vector2f(window.getSize().x, window.getSize().y)
        );
    }

    void update(float deltaTime) {
        if (background) {
            background->update(deltaTime);
        }
        // ... 其他更新邏輯 ...
    }

    void drawBackground() {
        if (background) {
            background->draw(window);
        }
    }

    // 在遊戲主循環中的繪製部分，首先繪製背景
    void draw() {
        drawBackground();
        // ... 繪製其他遊戲元素 ...
    }

    // 添加重置方法
    void reset() {
        bullets.clear();
        enemies.clear();
    }

    // 添加獲取敵人和子彈的方法
    const std::vector<Enemy>& getEnemies() const {
        return enemies;
    }

    const std::vector<Bullet>& getBullets() const {
        return bullets;
    }

    // 添加更新方法
    void updateBullets() {
        auto bulletIt = bullets.begin();
        while (bulletIt != bullets.end()) {
            bulletIt->update();  // 使用 Bullet 類的 update 方法，而不是直接使用 velocity
            
            bool bulletHit = false;
            auto enemyIt = enemies.begin();
            
            while (enemyIt != enemies.end()) {
                if (bulletIt->shape.getGlobalBounds().intersects(enemyIt->shape.getGlobalBounds())) {
                    (*killCountPtr)++;
                    (*goldPtr) += 1000;
                    
                    std::cout << "擊中敵人！當前金幣: " << *goldPtr << std::endl;
                    
                    enemyIt = enemies.erase(enemyIt);
                    bulletHit = true;
                    break;
                } else {
                    ++enemyIt;
                }
            }
            
            // 將 isOutOfBounds 檢查移到 Game 類內部
            bool outOfBounds = bulletIt->shape.getPosition().y < 0;
            
            if (bulletHit || outOfBounds) {
                bulletIt = bullets.erase(bulletIt);
            } else {
                ++bulletIt;
            }
        }
    }

    void updateEnemies() {
        for (auto& enemy : enemies) {
            enemy.update();
        }
    }

    // 添加子彈和敵人
    void addBullet(float x, float y) {
        Bullet bullet(x, y);
        bullets.push_back(bullet);
    }

    void addEnemy(float x, float y) {
        // 新的敵人邊界
        const float ENEMY_BOUNDARY_LEFT = 250.f;
        const float ENEMY_BOUNDARY_RIGHT = 950.f;
        const float ENEMY_WIDTH = 30.f;
        
        // 確保敵人在新的邊界內生成
        if (x < ENEMY_BOUNDARY_LEFT) {
            x = ENEMY_BOUNDARY_LEFT;
        }
        if (x > (ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH)) {
            x = ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH;
        }

        Enemy enemy(x, y);
        std::cout << "最終敵人位置X: " << x << std::endl;
        std::cout << "------------------------" << std::endl;
        enemies.push_back(enemy);
    }

    void removeEnemy(size_t index) {
        if (index < enemies.size()) {
            enemies.erase(enemies.begin() + index);
        }
    }

    // 修改 getEnemies 方法返回引用，這樣可以直接修改敵人容器
    std::vector<Enemy>& getEnemies() {
        return enemies;
    }

    // 修改檢測玩家碰撞的方法
    bool checkPlayerCollision(const Sprite& playerSprite) {
        for (const auto& enemy : enemies) {
            if (enemy.checkCollision(playerSprite)) {
                return true;
            }
        }
        return false;
    }
};

int main() {
    RenderWindow window(VideoMode(1200, 800), "SFML works!");
    srand(time(0));  // 初始化隨機數生成器
    
    // 加載玩家材質
    Texture playerTexture;
    if (!playerTexture.loadFromFile("/Users/cpcap/GTA6/texture/character/player.png")) {
        cout << "Error loading player texture!" << endl;
        cout << "Current working directory: " << filesystem::current_path() << endl;
        return -1;
    }
    
    // 創建玩家精靈替代原來的 CircleShape
    Sprite playerSprite(playerTexture);
    // 設置精靈原點為中心
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f);
    
    float x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在遊戲區域中心
    float y = 730.f;  // 原始值是 740.f，我們可以減小這個值來使角色往上移

    playerSprite.setPosition(x, y);
    
    // 在 main 函數中，修改玩家精靈的縮放比例
    float desiredWidth = 90.f;   // 期望的寬度
    float desiredHeight = 140.f;  // 期望的高度（可以調整這個值來改變高度）

    playerSprite.setScale(
        desiredWidth / playerTexture.getSize().x,
        desiredHeight / playerTexture.getSize().y
    );
    
    float moveSpeed = 0.2f;

    // 獲取玩家精靈的實際寬度（考慮縮放後的大小）
    float playerWidth = playerSprite.getGlobalBounds().width;

    // 設置移動邊界，考慮玩家寬度
    float leftBound = BOUNDARY_LEFT;                         // 左邊界
    float rightBound = BOUNDARY_LEFT + PLAY_AREA_WIDTH - playerWidth;  // 右邊界減去玩家寬度

    // 子彈容器
    std::vector<Bullet> bullets;
    bool spacePressed = false;

    // 添加血條
    RectangleShape healthBarBackground(Vector2f(200.f, 20.f));
    RectangleShape healthBar(Vector2f(200.f, 20.f));
    
    // 設置血條位置（右上角）
    healthBarBackground.setPosition(950.f, 50.f);
    healthBar.setPosition(950.f, 50.f);
    
    // 設置血條顏色和邊框
    healthBarBackground.setFillColor(Color(100, 100, 100));  // 灰色背景
    healthBar.setFillColor(Color::Green);                    // 綠色血條
    
    // 添加血條邊框
    healthBarBackground.setOutlineThickness(2.f);
    healthBarBackground.setOutlineColor(Color::White);
    
    // 設置血量
    float maxHealth = 100.f;
    float currentHealth = 100.f;

    // 敵人關變量
    std::vector<Enemy> enemies;
    Clock enemySpawnTimer;  // 用於計時生成敵人
    
    // 添加無敵時間計時器
    Clock invincibilityTimer;
    bool isInvincible = false;
    float invincibilityDuration = 1.0f;  // 1無敵時間

    // 在創建 Game 實例之前定義 killCount
    int killCount = 0;
    int gold = 30000;  // 初始金幣

    // 載入字體
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font!" << std::endl;
    }

    // 添加計數器文字
    sf::Text killCountText;
    killCountText.setFont(font);
    killCountText.setCharacterSize(24);
    killCountText.setFillColor(sf::Color::White);
    killCountText.setPosition(10.f, 10.f);
    killCountText.setString("Kills: 0");

    // 建遊戲實例
    Game game(window, &killCount, &gold);

    // 創建遊戲結束文字
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("Game Over!");
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(sf::Color::Red);
    
    // 創建提示文字
    sf::Text promptText;
    promptText.setFont(font);
    promptText.setString("Press R to Restart or ESC to Quit");
    promptText.setCharacterSize(30);
    promptText.setFillColor(sf::Color::White);
    
    // 設置文字位置
    gameOverText.setPosition(
        window.getSize().x/2 - gameOverText.getGlobalBounds().width/2,
        window.getSize().y/2 - gameOverText.getGlobalBounds().height/2 - 50
    );
    promptText.setPosition(
        window.getSize().x/2 - promptText.getGlobalBounds().width/2,
        window.getSize().y/2 + 50
    );

    // 添加遊戲狀態
    bool isGameOver = false;
    bool gameWon = false;

    // 添加子彈發射計時器
    Clock shootTimer;
    const float shootCooldown = 0.5f;  // 射擊冷卻時間（秒）

    // 添加敵人生成計時器
    const float enemySpawnInterval = 2.0f;  // 2秒生一個敵人

    // 創建勝利文字
    sf::Text gameWonText;
    gameWonText.setFont(font);
    gameWonText.setString("Victory!");
    gameWonText.setCharacterSize(50);
    gameWonText.setFillColor(sf::Color::Green);

    // 創建勝利提示文字
    sf::Text victoryPromptText;
    victoryPromptText.setFont(font);
    victoryPromptText.setString("Press R to Play Again or ESC to Quit");
    victoryPromptText.setCharacterSize(30);
    victoryPromptText.setFillColor(sf::Color::White);

    // 設置勝利文字位置
    gameWonText.setPosition(
        window.getSize().x/2 - gameWonText.getGlobalBounds().width/2,
        window.getSize().y/2 - gameWonText.getGlobalBounds().height/2 - 50
    );
    victoryPromptText.setPosition(
        window.getSize().x/2 - victoryPromptText.getGlobalBounds().width/2,
        window.getSize().y/2 + 50
    );

    // 在 main 函數開始處添加自動發射的計時器和間隔設置
    Clock autoShootTimer;  // 自動發射計時器
    const float autoShootInterval = 0.5f;  // 每0.5秒發射一次，你可以調整這個值

    sf::Clock clock;  // 添加時間來計算幀時間
    
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            
            // 添加調試模式的擊殺數增加
            if (!gameWon && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::J) {
                    killCount++;  // 每按一次J增加一個擊殺數
                    // 更新擊殺數顯示
                    killCountText.setString("Kills: " + std::to_string(killCount) + " | Gold: " + std::to_string(gold));
                }
            }
            
            // 遊戲結束時的按鍵處理
            if (isGameOver && event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::R)
                {
                    // 重置遊戲狀態
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(200.f, 20.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;
                    y = 730.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;  // 重置擊殺數
                    gold = 30000;   // 重置金幣數量
                    isGameOver = false;
                    
                    // 更新顯示文字
                    killCountText.setString("Kills: 0 | Gold: 30000");
                }
                else if (event.key.code == Keyboard::Escape)
                {
                    window.close();
                }
            }

            // 添加調試模式的按鍵檢測
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::H && !isGameOver) {
                    // 按H鍵扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    healthBar.setSize(Vector2f((currentHealth/maxHealth) * 200.f, 20.f));
                    
                    // 如果血量歸，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

            // 添加勝時的按鍵處理
            if (gameWon && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::R) {
                    // 重置遊戲狀態
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(200.f, 20.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;
                    y = 730.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;  // 重置擊殺數
                    gold = 30000;   // 重置金幣數量
                    gameWon = false;
                    
                    // 更新顯示文字
                    killCountText.setString("Kills: 0 | Gold: 30000");
                }
                else if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // 在遊戲循環中，修改碰撞檢測的部分
        if (!isInvincible) {
            auto enemyIt = game.getEnemies().begin();
            while (enemyIt != game.getEnemies().end()) {
                if (enemyIt->checkCollision(playerSprite)) {
                    // 扣血
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    healthBar.setSize(Vector2f((currentHealth/maxHealth) * 200.f, 20.f));
                    
                    // 設置無敵時間
                    isInvincible = true;
                    invincibilityTimer.restart();
                    
                    // 增加擊殺數和金幣
                    killCount++;
                    gold += 1000;  // 每擊敗一個敵人增加 1000 金幣
                    
                    // 立即更新 UI 文字
                    killCountText.setString("Kills: " + std::to_string(killCount) + " | Gold: " + std::to_string(gold));
                    
                    // 移除敵人
                    enemyIt = game.getEnemies().erase(enemyIt);

                    // 檢查是否達到勝利條件
                    if (killCount >= 10) {
                        gameWon = true;
                    }
                    
                    // 檢查是否死亡
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                    
                    break;
                } else {
                    ++enemyIt;
                }
            }
        }

        // 修改血量檢查邏輯
        if (currentHealth <= 0) {
            isGameOver = true;
        }

        window.clear();

        // 修改遊戲狀態檢查的邏輯
        if (!isGameOver && !gameWon) {  // 確保兩個狀態互斥
            game.update(deltaTime);  // 更新遊戲狀態，包括背景動畫
            game.drawBackground();   // 繪製背景
            
            // 繪製敵人
            for (const auto& enemy : game.getEnemies()) {
                window.draw(enemy.shape);
            }
            
            // 繪製玩家和子彈
            window.draw(playerSprite);
            for (const auto& bullet : game.getBullets()) {
                window.draw(bullet.shape);
            }
            
            // 繪製條
            window.draw(healthBarBackground);
            window.draw(healthBar);

            // 遊戲邏輯更新
            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                x = std::max(leftBound + playerWidth/2.f, x - moveSpeed);  // 考慮中心點偏移
            }
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                x = std::min(rightBound + playerWidth/2.f, x + moveSpeed);  // 考慮中心點偏移
            }
            
            // 檢查是否到達發射時間
            if (autoShootTimer.getElapsedTime().asSeconds() >= autoShootInterval) {
                // 從玩家中心位置發射子彈
                float bulletX = playerSprite.getPosition().x;
                float bulletY = playerSprite.getPosition().y - playerSprite.getGlobalBounds().height/2.f;
                
                game.addBullet(bulletX, bulletY);
                autoShootTimer.restart();  // 重置計時器
            }

            // 修改敵人生成邏輯
            if (enemySpawnTimer.getElapsedTime().asSeconds() >= enemySpawnInterval) {
                // 使用新的敵人邊界
                const float ENEMY_BOUNDARY_LEFT = 250.f;
                const float ENEMY_BOUNDARY_RIGHT = 950.f;
                const float ENEMY_WIDTH = 30.f;
                
                float randomX = ENEMY_BOUNDARY_LEFT + 
                    (static_cast<float>(rand()) / RAND_MAX) * 
                    (ENEMY_BOUNDARY_RIGHT - ENEMY_BOUNDARY_LEFT - ENEMY_WIDTH);
                
                if (randomX > (ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH)) {
                    randomX = ENEMY_BOUNDARY_RIGHT - ENEMY_WIDTH;
                }
                
                std::cout << "生成敵人位置X: " << randomX << std::endl;
                std::cout << "------------------------" << std::endl;
                
                game.addEnemy(randomX, 0.f);
                enemySpawnTimer.restart();
            }

            // 更新遊戲邏輯
            game.updateBullets();
            game.updateEnemies();
            playerSprite.setPosition(x, y);

            // 檢測玩家和敵人的碰撞
            if (!isInvincible) {
                if (game.checkPlayerCollision(playerSprite)) {
                    // 只血，不移除敵人
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    healthBar.setSize(Vector2f((currentHealth/maxHealth) * 200.f, 20.f));
                    isInvincible = true;
                    invincibilityTimer.restart();

                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

            // 更新無敵時間
            if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= invincibilityDuration) {
                isInvincible = false;
            }

            // 先檢查勝利條件
            if (killCount >= 10) {
                gameWon = true;  // 設置勝利狀態
                isGameOver = false;  // 確保不會觸發遊戲結束
            }
            // 再檢查失敗條件
            else if (currentHealth <= 0) {
                isGameOver = true;
                gameWon = false;
            }

            // 更新並繪製擊殺數
            killCountText.setString("Kills: " + std::to_string(killCount) + " | Gold: " + std::to_string(gold));
            window.draw(killCountText);
        }
        else if (gameWon) {
            // 繪製勝利畫面
            window.draw(gameWonText);
            window.draw(victoryPromptText);
            // 不繪製擊殺數和金幣
        }
        else if (isGameOver) {
            // 繪製遊戲結束畫面
            window.draw(gameOverText);
            window.draw(promptText);
            // 不繪製擊殺數和金幣
        }

        // 在遊戲結束畫面中顯示金幣數量
        sf::Text goldText("Gold: " + std::to_string(gold), font, 30);
        goldText.setFillColor(sf::Color::Black);
        goldText.setPosition(10, 40);  // 調整位置以顯示金幣

        window.display();
    }

    return 0;
}