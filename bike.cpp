#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

// 常量定義
const int windowWidth = 1200;
const int windowHeight = 800;
const int maxPlayerHealth = 5000;
const int maxEnemyHealth = 500;
const int maxBossMultiplier = 5; // BOSS 血量是普通怪物的 5 倍
const int maxActiveEnemies = 5;  // 每次最多存在的敵對生物數量
const float playerBulletSpeed = -0.5f; // 玩家子彈速度
const float enemyBulletSpeed = 0.3f;   // 敵人子彈速度
const int baseBulletDamage = 250;
const float baseMoveSpeed = 0.1f;

// 升級選項價格
const int healthUpgradeCost = 100;
const int damageUpgradeCost = 200;
const int speedUpgradeCost = 150;

struct Enemy {
    sf::CircleShape shape;
    int health;
    bool movingRight;
    bool isBoss;

    bool operator==(const Enemy& other) const {
        return this == &other; // 比較記憶體地址，確認是同一實例
    }
};

// 暫停功能
void showPauseScreen(sf::RenderWindow& window, sf::Font& font) {
    sf::Text pauseText("Game Paused", font, 50);
    pauseText.setFillColor(sf::Color::Blue);
    pauseText.setPosition(windowWidth / 2 - 150, windowHeight / 2 - 50);

    sf::Text instructionText("Press P to Resume", font, 30);
    instructionText.setFillColor(sf::Color::Black);
    instructionText.setPosition(windowWidth / 2 - 150, windowHeight / 2 + 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                return; // 按下 P 鍵繼續遊戲
            }
        }

        window.clear(sf::Color::White);
        window.draw(pauseText);
        window.draw(instructionText);
        window.display();
    }
}

// 顯示等待頁面與商店選單
void showShop(sf::RenderWindow& window, sf::Font& font, int& gold, int& playerHealth, int& bulletDamage, float& moveSpeed) {
    sf::Text shopTitle("Shop - Spend your Gold", font, 50);
    shopTitle.setFillColor(sf::Color::Blue);
    shopTitle.setPosition(windowWidth / 2 - 250, 100);

    sf::Text instruction("Press Space to Confirm, Up/Down to Navigate", font, 20);
    instruction.setFillColor(sf::Color::Black);
    instruction.setPosition(windowWidth / 2 - 250, 170);

    std::vector<std::string> options = {
        "Increase Health (+1000) - Cost: 100",
        "Increase Bullet Damage (+50) - Cost: 200",
        "Increase Move Speed (+0.05) - Cost: 150",
        "Exit Shop"
    };

    int selectedOption = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    selectedOption = (selectedOption - 1 + options.size()) % options.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    selectedOption = (selectedOption + 1) % options.size();
                } else if (event.key.code == sf::Keyboard::Space) {
                    if (selectedOption == 0 && gold >= healthUpgradeCost) {
                        playerHealth += 1000;
                        gold -= healthUpgradeCost;
                    } else if (selectedOption == 1 && gold >= damageUpgradeCost) {
                        bulletDamage += 50;
                        gold -= damageUpgradeCost;
                    } else if (selectedOption == 2 && gold >= speedUpgradeCost) {
                        moveSpeed += 0.05f;
                        gold -= speedUpgradeCost;
                    } else if (selectedOption == 3) {
                        return; // 退出商店
                    }
                }
            }
        }

        // 顯示商店選單
        window.clear(sf::Color::White);
        window.draw(shopTitle);
        window.draw(instruction);

        for (size_t i = 0; i < options.size(); ++i) {
            sf::Text optionText(options[i], font, 30);
            optionText.setFillColor(i == selectedOption ? sf::Color::Red : sf::Color::Black);
            optionText.setPosition(windowWidth / 2 - 300, 250 + i * 50);
            window.draw(optionText);
        }

        sf::Text goldText("Current Gold: " + std::to_string(gold), font, 30);
        goldText.setFillColor(sf::Color::Black);
        goldText.setPosition(windowWidth / 2 - 300, 450);
        window.draw(goldText);

        window.display();
    }
}

// 顯示關卡畫面
void showLevelScreen(sf::RenderWindow& window, sf::Font& font, const std::string& message, int& gold, int& playerHealth) {
    sf::Text levelText(message, font, 50);
    levelText.setFillColor(sf::Color::Blue);
    levelText.setPosition(windowWidth / 2 - 250, windowHeight / 2 - 50);

    sf::Text goldText("Gold: " + std::to_string(gold), font, 30);
    goldText.setFillColor(sf::Color::Black);
    goldText.setPosition(windowWidth / 2 - 200, windowHeight / 2 + 50);

    sf::Text healthText("Player Health: " + std::to_string(playerHealth), font, 30);
    healthText.setFillColor(sf::Color::Black);
    healthText.setPosition(windowWidth / 2 - 200, windowHeight / 2 + 100);

    sf::Text instructionText("Press Space to Continue", font, 30);
    instructionText.setFillColor(sf::Color::Black);
    instructionText.setPosition(windowWidth / 2 - 200, windowHeight / 2 + 150);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                return;
            }
        }

        window.clear(sf::Color::White);
        window.draw(levelText);
        window.draw(goldText);
        window.draw(healthText);
        window.draw(instructionText);
        window.display();
    }
}

int main() {
    // 初始化隨機數
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 創建視窗
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Square vs Enemies");

    // 字體
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: Could not load font!" << std::endl;
        return -1;
    }

    // 初始數據
    int playerHealth = maxPlayerHealth;
    int bulletDamage = baseBulletDamage;
    float moveSpeed = baseMoveSpeed;
    int gold = 30000;

    // 初始化文字
    sf::Text goldText("Gold: 0", font, 20);
    goldText.setFillColor(sf::Color::Black);
    goldText.setPosition(20, 80);

    sf::Text playerHealthText("Health: " + std::to_string(playerHealth) + "/" + std::to_string(maxPlayerHealth), font, 20);
    playerHealthText.setFillColor(sf::Color::Black);
    playerHealthText.setPosition(20, 50);

    sf::Text bossNameText("", font, 30);
    bossNameText.setFillColor(sf::Color::Magenta);
    bossNameText.setPosition(windowWidth / 2 - 150, 10);

    // BOSS 名稱
    std::vector<std::string> bossNames = {"rrro", "IM_Head", "syua_yuan_a_pei"};

    // 顯示遊戲開始畫面
    showLevelScreen(window, font, "Welcome to Square vs Enemies!", gold, playerHealth);

    // 主遊戲循環
    int currentLevel = 1;
    while (currentLevel <= 3 && window.isOpen()) {
        // 顯示關卡開始畫面
        showLevelScreen(window, font, "Level " + std::to_string(currentLevel) + " Starting...", gold, playerHealth);

        // 初始化關卡相關數據
        std::vector<sf::RectangleShape> playerBullets;
        std::vector<sf::RectangleShape> enemyBullets;
        std::vector<Enemy> enemies;
        int spawnedEnemies = 0, defeatedEnemies = 0;
        bool bossSpawned = false;
        int enemiesToSpawn = currentLevel == 1 ? 15 : (currentLevel == 2 ? 20 : 25);
        sf::Clock clock;

        sf::RectangleShape square(sf::Vector2f(100, 100));
        square.setFillColor(sf::Color::Red);
        square.setPosition(windowWidth / 2 - 50, windowHeight - 150);

        sf::RectangleShape leftBoundary(sf::Vector2f(5, windowHeight));
        leftBoundary.setFillColor(sf::Color::Black);
        leftBoundary.setPosition(200, 0);

        sf::RectangleShape rightBoundary(sf::Vector2f(5, windowHeight));
        rightBoundary.setFillColor(sf::Color::Black);
        rightBoundary.setPosition(windowWidth - 200, 0);

        sf::RectangleShape playerHealthBar(sf::Vector2f(300, 20));
        playerHealthBar.setFillColor(sf::Color::Green);
        playerHealthBar.setPosition(20, 20);

        // 遊戲內循環
        while (defeatedEnemies < enemiesToSpawn && playerHealth > 0 && window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                    showPauseScreen(window, font); // 暫停遊戲
                }
            }

            float deltaTime = clock.restart().asSeconds();

            // 玩家移動
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && square.getPosition().x > 200) {
                square.move(-moveSpeed, 0);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && square.getPosition().x < windowWidth - 200 - square.getSize().x) {
                square.move(moveSpeed, 0);
            }

            // 玩家子彈發射
            static float playerBulletCooldown = 0.4f;
            static float playerBulletTimer = 0.0f;
            playerBulletTimer += deltaTime;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && playerBulletTimer >= playerBulletCooldown) {
                sf::RectangleShape bullet(sf::Vector2f(10, 20));
                bullet.setFillColor(sf::Color::Green);
                bullet.setPosition(square.getPosition().x + square.getSize().x / 2 - 5, square.getPosition().y);
                playerBullets.push_back(bullet);
                playerBulletTimer = 0.0f;
            }

            for (auto& bullet : playerBullets) {
                bullet.move(0, playerBulletSpeed);
            }

            // 敵人生成邏輯
            if (spawnedEnemies < enemiesToSpawn && enemies.size() < maxActiveEnemies) {
                Enemy newEnemy;
                newEnemy.shape = sf::CircleShape(50);
                newEnemy.shape.setFillColor(sf::Color::Blue);
                newEnemy.shape.setPosition(200 + std::rand() % (windowWidth - 400), 50);
                newEnemy.health = maxEnemyHealth;
                newEnemy.movingRight = std::rand() % 2 == 0;
                newEnemy.isBoss = false;
                enemies.push_back(newEnemy);
                ++spawnedEnemies;

                // 生成 BOSS
                if (!bossSpawned && spawnedEnemies >= enemiesToSpawn / 2) {
                    Enemy boss;
                    boss.shape = sf::CircleShape(70);
                    boss.shape.setFillColor(sf::Color::Magenta);
                    boss.shape.setPosition(windowWidth / 2 - 70, 50);
                    boss.health = maxEnemyHealth * maxBossMultiplier;
                    boss.movingRight = true;
                    boss.isBoss = true;
                    enemies.push_back(boss);
                    bossNameText.setString("BOSS: " + bossNames[currentLevel - 1]);
                    bossSpawned = true;
                }
            }

            for (auto& enemy : enemies) {
                if (enemy.movingRight) {
                    enemy.shape.move(0.1f, 0);
                    if (enemy.shape.getPosition().x + enemy.shape.getRadius() * 2 >= windowWidth - 200) {
                        enemy.movingRight = false;
                    }
                } else {
                    enemy.shape.move(-0.1f, 0);
                    if (enemy.shape.getPosition().x <= 200) {
                        enemy.movingRight = true;
                    }
                }
            }

            // 敵人子彈發射邏輯
            static float enemyBulletCooldown = 2.0f;
            static float enemyBulletTimer = 0.0f;
            enemyBulletTimer += deltaTime;
            if (enemyBulletTimer >= enemyBulletCooldown) {
                for (const auto& enemy : enemies) {
                    sf::RectangleShape bullet(sf::Vector2f(10, 20));
                    bullet.setFillColor(sf::Color::Red);
                    bullet.setPosition(enemy.shape.getPosition().x + enemy.shape.getRadius() - 5,
                                        enemy.shape.getPosition().y + enemy.shape.getRadius() * 2);
                    enemyBullets.push_back(bullet);
                }
                enemyBulletTimer = 0.0f;
            }

            for (auto& bullet : enemyBullets) {
                bullet.move(0, enemyBulletSpeed);
            }

            // 碰撞檢測
            for (auto it = playerBullets.begin(); it != playerBullets.end();) {
                bool bulletHit = false;
                for (auto& enemy : enemies) {
                    if (it->getGlobalBounds().intersects(enemy.shape.getGlobalBounds())) {
                        enemy.health -= bulletDamage;
                        if (enemy.health <= 0) {
                            if (enemy.isBoss) {
                                bossNameText.setString("");
                            }
                            enemies.erase(std::remove(enemies.begin(), enemies.end(), enemy), enemies.end());
                            ++defeatedEnemies;
                            gold += 50;
                        }
                        bulletHit = true;
                        break;
                    }
                }
                if (bulletHit) {
                    it = playerBullets.erase(it);
                } else {
                    ++it;
                }
            }

            for (auto it = enemyBullets.begin(); it != enemyBullets.end();) {
                if (it->getGlobalBounds().intersects(square.getGlobalBounds())) {
                    playerHealth -= 200;
                    it = enemyBullets.erase(it);
                } else {
                    ++it;
                }
            }

            // 更新血量條與金幣顯示
            playerHealthText.setString("Health: " + std::to_string(playerHealth) + "/" + std::to_string(maxPlayerHealth));
            goldText.setString("Gold: " + std::to_string(gold));
            playerHealthBar.setSize(sf::Vector2f(300 * (static_cast<float>(playerHealth) / maxPlayerHealth), 20));

            // 繪製
            window.clear(sf::Color::White);
            window.draw(leftBoundary);
            window.draw(rightBoundary);
            window.draw(playerHealthBar);
            window.draw(playerHealthText);
            window.draw(goldText);
            window.draw(bossNameText);
            window.draw(square);
            for (const auto& bullet : playerBullets) {
                window.draw(bullet);
            }
            for (const auto& bullet : enemyBullets) {
                window.draw(bullet);
            }
            for (const auto& enemy : enemies) {
                window.draw(enemy.shape);
            }
            window.display();

            if (playerHealth <= 0) {
                showLevelScreen(window, font, "Game Over!", gold, playerHealth);
                return 0;
            }
        }

        if (playerHealth > 0 && currentLevel != 3) {
            showShop(window, font, gold, playerHealth, bulletDamage, moveSpeed);
        }

        ++currentLevel;
    }

    // 遊戲結束畫面
    showLevelScreen(window, font, "Victory! Thanks for Playing!", gold, playerHealth);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        return 0;
    }
    //return 0;
}

