#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>  // 為了使用 rand()
#include <ctime>    // 為了使用 time()
#include <iostream>  // 添加這行
#include <filesystem>  // 添加這行
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

class Game {
private:
    RenderWindow& window;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int* killCountPtr;  // 指向擊殺計數的指針

public:
    Game(RenderWindow& win, int* killCount) : window(win), killCountPtr(killCount) {}

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
        // 更新子彈位置
        for (auto& bullet : bullets) {
            bullet.update();
        }

        // 檢查子彈和敵人的碰撞
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
            bool bulletHit = false;
            
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                if (bulletIt->shape.getGlobalBounds().intersects(enemyIt->shape.getGlobalBounds())) {
                    // 子彈擊中敵人
                    enemyIt = enemies.erase(enemyIt);
                    bulletHit = true;
                    (*killCountPtr)++;  // 增加擊殺計數
                    break;
                } else {
                    ++enemyIt;
                }
            }
            
            if (bulletHit || bulletIt->shape.getPosition().y < 0) {
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
        // 最後的安全檢查
        const float BOUNDARY_RIGHT = 1000.f;
        const float ENEMY_WIDTH = 30.f;
        
        if (x > (BOUNDARY_RIGHT - ENEMY_WIDTH)) {
            x = BOUNDARY_RIGHT - ENEMY_WIDTH;
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
    if (!playerTexture.loadFromFile("/Users/cpcap/GTA6/texture/player.png")) {
        cout << "Error loading player texture!" << endl;
        cout << "Current working directory: " << filesystem::current_path() << endl;
        return -1;
    }
    
    // 創建玩家精靈替代原來的 CircleShape
    Sprite playerSprite(playerTexture);
    // 設置精靈的原點為中心
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f);
    
    float x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在遊戲區域中心
    float y = 740.f;
    playerSprite.setPosition(x, y);
    
    // 如果需要縮放精靈到特定大小（例如原來圓形的大小）
    playerSprite.setScale(
        60.f / playerTexture.getSize().x,  // 60.f 是原來圓形的直徑
        60.f / playerTexture.getSize().y
    );
    
    float moveSpeed = 0.5f;
    float leftBound = BOUNDARY_LEFT;
    float rightBound = BOUNDARY_LEFT + PLAY_AREA_WIDTH - 60.f;

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

    // 敵人相關變量
    std::vector<Enemy> enemies;
    Clock enemySpawnTimer;  // 用於計時生成敵人
    
    // 添加無敵時間計時器
    Clock invincibilityTimer;
    bool isInvincible = false;
    float invincibilityDuration = 1.0f;  // 1無敵時間

    // 在創建 Game 實例之前定義 killCount
    int killCount = 0;

    // 載入字體
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        std::cout << "Error loading font!" << std::endl;
        return -1;
    }

    // 添加計數器文字
    sf::Text killCountText;
    killCountText.setFont(font);
    killCountText.setCharacterSize(24);
    killCountText.setFillColor(sf::Color::White);
    killCountText.setPosition(10, 10);
    killCountText.setString("Kills: 0");

    // 創建遊戲實例
    Game game(window, &killCount);

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

    // 在 main 函數開始處，window 創建之後添加邊界定義
    RectangleShape leftBoundary;
    RectangleShape rightBoundary;

    // 設置邊界的大小
    leftBoundary.setSize(Vector2f(2.f, 800.f));
    rightBoundary.setSize(Vector2f(2.f, 800.f));

    // 使用正確的常量設置位置
    leftBoundary.setPosition(BOUNDARY_LEFT, 0.f);                    // x = 200
    rightBoundary.setPosition(BOUNDARY_LEFT + PLAY_AREA_WIDTH, 0.f); // x = 1000

    // 設置邊界顏色
    leftBoundary.setFillColor(Color::White);
    rightBoundary.setFillColor(Color::White);

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

    // 主遊戲循環
    while (window.isOpen())
    {
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
                    killCountText.setString("Kills: " + std::to_string(killCount));
                }
            }
            
            // 遊戲結束時的按鍵處理
            if (isGameOver && event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::R)
                {
                    // 修改重置邏輯
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(200.f, 20.f));
                    x = BOUNDARY_LEFT + PLAY_AREA_WIDTH/2;  // 在遊戲區域中心
                    y = 740.f;
                    playerSprite.setPosition(x, y);
                    game.reset();  // 使用重置方法替代重新創建實例
                    isGameOver = false;
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
                    
                    // 如果血量歸零，觸發遊戲結束
                    if (currentHealth <= 0) {
                        isGameOver = true;
                    }
                }
            }

            // 添加勝利時的按鍵處理
            if (gameWon && event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::R) {
                    // 重置遊戲
                    currentHealth = maxHealth;
                    healthBar.setSize(Vector2f(200.f, 20.f));
                    x = 600.f - 30.f;
                    y = 740.f;
                    playerSprite.setPosition(x, y);
                    game.reset();
                    killCount = 0;  // 重置擊殺數
                    gameWon = false;
                }
                else if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // 檢測碰撞後的血量檢查
        if (!isInvincible) {
            for (size_t i = 0; i < game.getEnemies().size(); i++) {
                if (game.getEnemies()[i].checkCollision(playerSprite)) {
                    currentHealth = std::max(0.f, currentHealth - 10.f);
                    healthBar.setSize(Vector2f((currentHealth/maxHealth) * 200.f, 20.f));
                    isInvincible = true;
                    invincibilityTimer.restart();
                    killCount++;
                    break;
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
            // 繪製邊界（在繪製其他物件之前）
            window.draw(leftBoundary);
            window.draw(rightBoundary);
            
            // 繪製敵人
            for (const auto& enemy : game.getEnemies()) {
                window.draw(enemy.shape);
            }
            
            // 繪製玩家和子彈
            window.draw(playerSprite);
            for (const auto& bullet : game.getBullets()) {
                window.draw(bullet.shape);
            }
            
            // 繪製血條
            window.draw(healthBarBackground);
            window.draw(healthBar);

            // 遊戲邏輯更新
            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                x = std::max(leftBound, x - moveSpeed);
            }
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                x = std::min(rightBound, x + moveSpeed);
            }
            
            // 處理射擊
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                if (shootTimer.getElapsedTime().asSeconds() >= shootCooldown) {
                    // 從玩位置發射子彈
                    game.addBullet(playerSprite.getPosition().x + playerSprite.getGlobalBounds().width / 2.f, 
                                 playerSprite.getPosition().y);
                    shootTimer.restart();
                }
            }

            // 修改敵人生成邏輯
            if (enemySpawnTimer.getElapsedTime().asSeconds() >= enemySpawnInterval) {
                // 計算生成範圍（確保不超過1000）
                float randomX = BOUNDARY_LEFT + 
                    (static_cast<float>(rand()) / RAND_MAX) * 
                    (BOUNDARY_RIGHT - BOUNDARY_LEFT - ENEMY_WIDTH);
                
                // 最後的安全檢查
                if (randomX > (BOUNDARY_RIGHT - ENEMY_WIDTH)) {
                    randomX = BOUNDARY_RIGHT - ENEMY_WIDTH;
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
            killCountText.setString("Kills: " + std::to_string(killCount));
            window.draw(killCountText);
        }
        else if (gameWon) {
            // 繪製勝利畫面
            window.draw(gameWonText);
            window.draw(victoryPromptText);
            window.draw(killCountText);
        }
        else if (isGameOver) {
            // 繪製遊戲結束畫面
            window.draw(gameOverText);
            window.draw(promptText);
            window.draw(killCountText);
        }

        window.display();
    }

    return 0;
}