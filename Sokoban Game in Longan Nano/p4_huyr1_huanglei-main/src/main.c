#include "lcd/lcd.h"
#include "lcd/pics.h"
#include <string.h>
#include "utils.h"
#include "stdio.h"

void Inp_init(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);

    gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6);
    gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
}

void IO_init(void)
{
    Inp_init(); // inport init
    Lcd_Init(); // LCD init
}


//find index of a thing. return -1 if not found. if multiple found, return the first one (ie the highest z)
int FindIndex(short logincalX, short logicalY, GameConfig conf){
    if(logincalX < 0 || logincalX >= 20 || logicalY < 0 || logicalY >= 10){
        return -1;
    }
    int i;
    for(i = 0; i < conf.objectCount; i++){
        if(conf.objects[i].logincalX == logincalX && conf.objects[i].logincalY == logicalY){
            return i;
        }
    }
    return -1;
}

//find whether there is goal below
bool isOnGoal(short logincalX,short logicalY, GameConfig conf){
    if(logincalX < 0 || logincalX >= 20 || logicalY < 0 || logicalY >= 10){
        return 0;
    }
    for(int i = 0; i < conf.objectCount; i++){
        if(conf.objects[i].logincalX == logincalX && conf.objects[i].logincalY == logicalY && conf.objects[i].type == goal){
            return 1;
        }
    }
    return 0;
}


// direction: 0: left, 1: down, 2: up, 3: right
// return 0 if game over, 1 if game continues. use incremental rendering
int runGameOneRound(int direction, GameConfig conf){
    static int dx[] = {-1,0,0,1};
    static int dy[] = {0,1,-1,0};
    static int associatedIndexes[5];
    int associatedCount = 0;
    int X = conf.objects[conf.playerIndex].logincalX;
    int Y = conf.objects[conf.playerIndex].logincalY;
    int nextX = conf.objects[conf.playerIndex].logincalX + dx[direction];
    int nextY = conf.objects[conf.playerIndex].logincalY + dy[direction];
    // slowDebugf("X %d  ", conf.objects[conf.playerIndex].logincalX);
    // slowDebugf("Y %d  ", conf.objects[conf.playerIndex].logincalY);
    //find other on its way
    int otherIndex = FindIndex(nextX, nextY, conf);
    // slowDebugf("otherIndex %d", otherIndex);
    //if other is out of bound
    if(otherIndex == -1) 
        return 1;
    //if other is a wall
    if(conf.objects[otherIndex].type == wall){
        return 1;
    }
    //if other is a box
    if(conf.objects[otherIndex].type == box || conf.objects[otherIndex].type == boxOnGoal){
        int nextBoxX = nextX + dx[direction];
        int nextBoxY = nextY + dy[direction];
        //find other on its way
        int otherBoxIndex = FindIndex(nextBoxX, nextBoxY, conf);
        //if other is out of bound
        if(otherBoxIndex == -1) 
            return 1;
        //if other is a wall
        if(conf.objects[otherBoxIndex].type == wall){
            return 1;
        }
        //if other is a box
        if(conf.objects[otherBoxIndex].type == box || conf.objects[otherBoxIndex].type == boxOnGoal){
            return 1;
        }
        //if other is a goal or empty
        if(conf.objects[otherBoxIndex].type == goal || conf.objects[otherBoxIndex].type == empty){
            //move box
            conf.objects[otherIndex] = createBox(nextBoxX, nextBoxY);
            associatedIndexes[associatedCount++] = otherIndex;
            //move player
            conf.objects[conf.playerIndex] = createPlayer(nextX, nextY);
            associatedIndexes[associatedCount++] = conf.playerIndex;
            //find whatever is on the player's original position
            int otherPlayerIndex = FindIndex(X, Y, conf);
            associatedIndexes[associatedCount++] = otherPlayerIndex;
            //see if it's on goal
            if(isOnGoal(nextX,nextY,conf)){
                conf.objects[conf.playerIndex].type = playerOnGoal;
            }else conf.objects[conf.playerIndex].type = player;
            //see if box is on goal
            if(isOnGoal(nextBoxX,nextBoxY,conf)){
                conf.objects[otherIndex].type = boxOnGoal;
            }else conf.objects[otherIndex].type = box;

            //render
            for(int t = 0; t < associatedCount; t++){
                renderObject(conf.objects[associatedIndexes[t]]);
            }
            return 1;
        }
    }
    //if other is a goal or empty
    if(conf.objects[otherIndex].type == goal || conf.objects[otherIndex].type == empty){
        //move player
        conf.objects[conf.playerIndex] = createPlayer(nextX, nextY);
        associatedIndexes[associatedCount++] = conf.playerIndex;
        //find whatever is on the player's original position
        int otherPlayerIndex = FindIndex(X, Y, conf);
        associatedIndexes[associatedCount++] = otherPlayerIndex;
        // debugf("otherPlayerIndex: %d", otherPlayerIndex);
        // slowDebugf("otherPlayerType %d", conf.objects[otherPlayerIndex].type);
        //render
        //see if it's on goal
        if(isOnGoal(nextX,nextY,conf)){
            conf.objects[conf.playerIndex].type = playerOnGoal;
        }else conf.objects[conf.playerIndex].type = player;

        for(int t = 0; t < associatedCount; t++){
            renderObject(conf.objects[associatedIndexes[t]]);
        }
        return 1;
    }
    return 0;
}


void mergesortObjectsByZ(Object* objectsBegin, Object* objectsEnd){
    //sort objects by z
    int n = objectsEnd - objectsBegin;
    if(n <= 1) return;
    int mid = n / 2;
    mergesortObjectsByZ(objectsBegin, objectsBegin + mid);
    mergesortObjectsByZ(objectsBegin + mid, objectsEnd);
    Object* temp = (Object*)malloc(sizeof(Object) * n);
    int i = 0, j = mid, k = 0;
    while(i < mid && j < n){
        if(objectsBegin[i].renderZ > objectsBegin[j].renderZ){
            temp[k++] = objectsBegin[i++];
        }
        else{
            temp[k++] = objectsBegin[j++];
        }
    }
    while(i < mid){
        temp[k++] = objectsBegin[i++];
    }
    while(j < n){
        temp[k++] = objectsBegin[j++];
    }
    for(int i = 0; i < n; i++){
        objectsBegin[i] = temp[i];
    }
    free(temp);
}

void showStep(int step){
    if(step < 10)
        LCD_ShowNum(0,0,step,1,WHITE);
    else if(step < 100)
        LCD_ShowNum(0,0,step,2,WHITE);
    LCD_ShowNum(0,0,step,3,WHITE);
}

//init game scene
void initGameInstanceScene(GameConfig conf){
    //sort objects by z
    mergesortObjectsByZ(conf.objects, conf.objects + conf.objectCount);
    //render in reverse order
    debug("init instance");
    for(int i = conf.objectCount - 1; i >= 0; i--){
        delay_1ms(10);
        renderObject(conf.objects[i]);
    }
}

bool checkGameOver(GameConfig conf){
    for(int i = 0; i < conf.objectCount; i++){
        if(conf.objects[i].type == box){
            return 0;
        }
    }
    return 1;
}


int runGameInstance(GameConfig conf,int* ops){
    int health = 1,step = 0;
    while(health){
        if(Get_Button(JOY_LEFT)){
            showStep(step++);
            // debug("Get_Button(JOY_LEFT)");
            health = runGameOneRound(0, conf);
            ops[step - 1] = 0;
        }
        else if(Get_Button(JOY_DOWN)){
            showStep(step++);
            // debug("Get_Button(JOY_DOWN)");
            health = runGameOneRound(1, conf);
            ops[step - 1] = 1;
        }
        else if(Get_Button(JOY_CTR)){
            showStep(step++);
            // debug("Get_Button(JOY_CTR)");
            health = runGameOneRound(2, conf);
            ops[step - 1] = 2;
        }
        else if(Get_Button(JOY_RIGHT)){
            showStep(step++);
            // debug("Get_Button(JOY_RIGHT)");
            health = runGameOneRound(3, conf);
            ops[step - 1] = 3;
        }
        if(step >= 999)
            break;
        if(health == 1 && checkGameOver(conf)){
            break;
        }
        delay_1ms(40);
        // initGameInstanceScene(conf);
    }
    return step;
}


void gameOverScreen(GameConfig confOld,GameConfig confnew,int* ops,int len){
    initGameInstanceScene(confnew);
    if(len >= 999){
        LCD_ShowString(160/2,80/2,"GG",YELLOW);
    }else 
        LCD_ShowString(160/2,80/2,"You Won",YELLOW);
    for(int t = 0; t < len; ++t){
        if(len >= 999){
            LCD_ShowString(160/2,80/2,"GG",YELLOW);
        }else 
            LCD_ShowString(160/2,80/2,"You Won",YELLOW);
        runGameOneRound(ops[t],confnew);
        delay_1ms(20);
    }
    delay_1ms(1000);
}


GameConfig getGameConfigLevel_1(int num){
    debug("getGameConfigLevel_1");
    GameConfig config;
    config.level = 1;
    config.objectCount = 0;
    config.playerIndex = 0;
    config.objects = (Object *)calloc(250, sizeof(Object));
    for (int i = 0; i < 20;i++){
        for (int j = 0; j < 10;j++){
            config.objects[config.objectCount++] = createEmpty(i, j);
        }
    }
    for (int i = 0; i < 10;i++){
        config.objects[config.objectCount++] = createWall(i, 3);
        config.objects[config.objectCount++] = createWall(i, 5);
        config.objects[config.objectCount++] = createWall(i, 7);
    }
    for (int i = 0; i < num;i++){
        config.objects[config.objectCount++] = createBox(2 * i + 1, 8);
    }
    for (int i = 0; i < num; i++)
    {
        config.objects[config.objectCount++] = createGoal(i, 4);
    }
    config.objects[config.objectCount++] = createPlayer(19, 6);
    debug("getGameConfigLevel_1 return");
    return config;
}
GameConfig getGameConfigLevel_2(int num){
    GameConfig config;
    config.level = 2;
    config.objectCount = 0;
    config.playerIndex = 0;
    config.objects = (Object *)calloc(280, sizeof(Object));
    for (int i = 0; i < 20;i++){
        for (int j = 0; j < 10;j++){
            config.objects[config.objectCount++] = createEmpty(i, j);
        }
    }
    for (int i = 0; i < 8;i++){
        config.objects[config.objectCount++] = createWall(2, i);
        config.objects[config.objectCount++] = createWall(18, i);
    }
    for (int i = 0; i < 15;i++){
        config.objects[config.objectCount++] = createWall(i + 3, 0);
    }
    for (int i = 0; i < 7;i++){
        config.objects[config.objectCount++] = createWall(i + 3, 7);
        config.objects[config.objectCount++] = createWall(i + 11, 7);
    }
    for (int i = 0; i < num;i++){
        config.objects[config.objectCount++] = createGoal(8 + i, 5);
        config.objects[config.objectCount++] = createBox(2 * i + 1, 8);
    }
    config.objects[config.objectCount++] = createPlayer(19, 9);
    return config;
}

GameConfig getGameConfigLevel_3(int num){
    GameConfig config;
    config.level = 3;
    config.objectCount = 0;
    config.playerIndex = 0;
    config.objects = (Object *)calloc(250, sizeof(Object));
    for (int i = 0; i < 20;i++){
        for (int j = 0; j < 10;j++){
            config.objects[config.objectCount++] = createEmpty(i, j);
        }
    }
    for (int i = 2; i < 10;i++){
        config.objects[config.objectCount++] = createWall(3, i);
        config.objects[config.objectCount++] = createWall(5, i);
        config.objects[config.objectCount++] = createWall(7, i);
    }
    for (int i = 0; i < num;i++){
        config.objects[config.objectCount++] = createGoal(4, i + 2);
        config.objects[config.objectCount++] = createBox(10 + i, 2 * i + 1);
    }
    config.objects[config.objectCount++] = createPlayer(19, 1);
    return config;
}

GameConfig getWelcome(){
    GameConfig config;
    config.level = 3;
    config.objectCount = 0;
    config.playerIndex = 0;
    config.objects = (Object *)calloc(250, sizeof(Object));
    for (int i = 0; i < 20;i++){
        for (int j = 0; j < 10;j++){
            config.objects[config.objectCount++] = createEmpty(i, j);
        }
    }
    for(int t=0;t<40;++t)
        config.objects[config.objectCount++] = createGoal(rand()%20, rand()%10);
    config.objects[config.objectCount++] = createPlayer(19, 1);
    return config;
}

void welcomeScreen(void){
    LCD_ShowImage(0,0,160 - 1,80 - 1,welcome);
    delay_1ms(2000);
    //show string "Sokoban" in the middle
    GameConfig config = getWelcome();
    for(int t = 1 ; t < 800; ++t){
        LCD_ShowString(160/2,80/2,"Sokoban",YELLOW);
        runGameOneRound(rand()%4,config);
        delay_1ms(10);
    }
    free(config.objects);
}

GameConfig selectLevel(){
    int level = 1, num = 1;
    LCD_Clear(BLACK);
    while(1){
        if(Get_Button(JOY_LEFT)){
            num -= 1;
        }else if(Get_Button(JOY_RIGHT)){
            num += 1;
        }else if(Get_Button(JOY_DOWN)){
            level -= 1;
        }else if(Get_Button(JOY_CTR)){
            level += 1;
        }else if(Get_BOOT0())
            break;
        if(level > 3)
            level = 3;
        if(level <= 0)
            level = 1;
        if(num > 4)
            num = 4;
        if(num <= 0)
            num = 1;
        LCD_ShowString(0,20,"Level",BLUE);
        LCD_ShowNum(80,20,level,1,BLUE);
        LCD_ShowString(0,60,"num",BLUE);
        LCD_ShowNum(80,60,num,1,BLUE);
        delay_1ms(20);
    }
    if(level == 1){
        return getGameConfigLevel_1(num);
    }
    if(level == 2){
        return getGameConfigLevel_2(num);
    }
    if(level == 3){
        return getGameConfigLevel_3(num);
    }
    return getGameConfigLevel_1(num);
}

int main(void)
{
    IO_init();         // init OLED
    // YOUR CODE HERE
    static int ops[1000];
    srand(114514);
    while(1){
        int steps = 0;
        welcomeScreen();
        GameConfig config = selectLevel();
        GameConfig bakup = config;
        bakup.objects = calloc(config.objectCount,sizeof(Object));
        memcpy(bakup.objects,config.objects,sizeof(Object) * config.objectCount);
        debug("before init");
        initGameInstanceScene(config);
        steps = runGameInstance(config,ops);
        gameOverScreen(config,bakup,ops,steps);
        delay_1ms(2000);
        debug("free init");
        free(config.objects);
        free(bakup.objects);
    }
    // END OF YOUR CODE
    return 0;
    while (1)
    {
        // LCD_Clear(BLACK);
        LCD_ShowString(60,25,"TEST",WHITE);
        if (Get_Button(JOY_LEFT))
        {
            LCD_ShowString(5,25,"L", BLUE);
            //continue;
        }
        if (Get_Button(JOY_DOWN))
        {
            LCD_ShowString(25,45,"D", BLUE);
        }
        LCD_ShowString(5,5,"U:INOP",RED);
        if (Get_Button(JOY_RIGHT))
        {
            LCD_ShowString(45,25,"R", BLUE);
        }
        if (Get_Button(JOY_CTR))
        {
            LCD_ShowString(25,25,"C", BLUE);
        }
        if (Get_Button(BUTTON_1))
        {
            LCD_ShowString(60,5,"SW1", BLUE);
        }
        LCD_ShowString(60,45,"SW2:INOP",RED);
        delay_1ms(100);
    }
}
