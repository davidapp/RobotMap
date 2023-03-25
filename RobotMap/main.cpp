#include <iostream>
#include "Robot.h"
#include <vector>

#define MAP_SIZE 10
int map[MAP_SIZE][MAP_SIZE] = { 0 };

#define ROBOT_CNT 5
Robot robot[ROBOT_CNT];

std::vector<Robot> planList;
std::vector<Robot> runningList;
std::vector<int> removeID;

void printmap()
{
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            printf("%2d ", map[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void printtask() {
    for (int i = 0; i < planList.size(); i++) {
        Robot r = planList[i];
        printf("robot %d at (%d,%d) wants to go to (%d,%d)\n", r.m_id, r.m_xpos,
            r.m_ypos, r.m_xtarget, r.m_ytarget);
    }
    printf("\n");
}

int checkmap(int fromx, int fromy, int tox, int toy, int rid) {
    int xstep = 0;
    if (fromx < tox) xstep = 1;
    else if (fromx > tox) xstep = -1;

    int ystep = 0;
    if (fromy < toy) ystep = 1;
    else if (fromy > toy) ystep = -1;

    bool bOK = true;
    // 先移动 x 再移动 y
    for (int x = fromx; x != tox + xstep; x += xstep) {
        if (map[x][fromy] != 0 && map[x][fromy] != rid) bOK = false;
    }

    for (int y = fromy; y != toy + ystep; y += ystep) {
        if (map[tox][y] != 0 && map[tox][y] != rid) bOK = false;
    }
    if (bOK) return 1;

    // 如果不行，再试试先 y 再 x
    bOK = true;
    for (int y = fromy; y != toy + ystep; y += ystep) {
        if (map[fromx][y] != 0 && map[fromx][y] != rid) bOK = false;
    }
    for (int x = fromx; x != tox + xstep; x += xstep) {
        if (map[x][toy] != 0 && map[x][toy] != rid) bOK = false;
    }
    if (bOK) return 2;

    return 0;
}

void lockmap(int fromx, int fromy, int tox, int toy, int rid, int lockway) {
    int xstep = 0;
    if (fromx < tox) xstep = 1;
    else if (fromx > tox) xstep = -1;

    int ystep = 0;
    if (fromy < toy) ystep = 1;
    else if (fromy > toy) ystep = -1;

    if (lockway == 1) {
        for (int x = fromx; x != tox + xstep; x += xstep) {
            map[x][fromy] = rid;
        }

        for (int y = fromy; y != toy + ystep; y += ystep) {
            map[tox][y] = rid;
        }
    }
    else if (lockway == 2) {
        for (int y = fromy; y != toy + ystep; y += ystep) {
            map[fromx][y] = rid;
        }
        for (int x = fromx; x != tox + xstep; x += xstep) {
            map[x][toy] = rid;
        }
    }
}

void unlockmap(int fromx, int fromy, int tox, int toy, int rid, int lockway) {
    int xstep = 0;
    if (fromx < tox) xstep = 1;
    else if (fromx > tox) xstep = -1;

    int ystep = 0;
    if (fromy < toy) ystep = 1;
    else if (fromy > toy) ystep = -1;

    if (lockway == 1) {
        for (int x = fromx; x != tox + xstep; x += xstep) {
            map[x][fromy] = 0;
        }

        for (int y = fromy; y != toy + ystep; y += ystep) {
            map[tox][y] = 0;
        }
    }
    else if (lockway == 2) {
        for (int y = fromy; y != toy + ystep; y += ystep) {
            map[fromx][y] = 0;
        }
        for (int x = fromx; x != tox + xstep; x += xstep) {
            map[x][toy] = 0;
        }
    }

    map[tox][toy] = rid;
}


void set_random_init_pos(int robotindex) {
    bool bfind = false;
    int xpos = 0;
    int ypos = 0;
    do {
        // 生成一个随机边沿坐标
        xpos = rand() % MAP_SIZE;
        ypos = rand() % MAP_SIZE;
        int dir = rand() % 4;
        if (dir == 0) xpos = 0;
        else if (dir == 1) xpos = MAP_SIZE - 1;
        else if (dir == 2) ypos = 0;
        else if (dir == 3) ypos = MAP_SIZE - 1;
        if (map[xpos][ypos] == 0) bfind = true;
    } while (!bfind);
    // 找到后设置坐标
    robot[robotindex].m_xpos = xpos;
    robot[robotindex].m_ypos = ypos;
    map[xpos][ypos] = robot[robotindex].m_id;
}

void set_random_target_pos(int robotindex) {
    bool bfind = false;
    int xpos = 0;
    int ypos = 0;
    do {
        // 生成一个随机边沿坐标，只能去另外 3 边，否则会造成 A 想去 B, B 想去 A，结果死锁了。
        xpos = rand() % MAP_SIZE;
        ypos = rand() % MAP_SIZE;
        int dir = rand() % 4;
        if (dir == 0) xpos = 0;
        else if (dir == 1) xpos = MAP_SIZE - 1;
        else if (dir == 2) ypos = 0;
        else if (dir == 3) ypos = MAP_SIZE - 1;
        if (xpos == robot[robotindex].m_xpos || ypos == robot[robotindex].m_ypos) continue;
        // 不要去 4 个角上
        if ((xpos == 0 && ypos == MAP_SIZE - 1) || (ypos == 0 && xpos == MAP_SIZE - 1)) continue;
        if ((xpos == 0 && ypos == 0) || (ypos == MAP_SIZE - 1 && xpos == MAP_SIZE - 1)) continue;
        if (map[xpos][ypos] == 0) bfind = true;
    } while (!bfind);
    // 找到后设置坐标
    robot[robotindex].m_xtarget = xpos;
    robot[robotindex].m_ytarget = ypos;
}

void init()
{
    for (int i = 0; i < ROBOT_CNT; i++) {
        robot[i].m_id = i + 1;
        set_random_init_pos(i);
        set_random_target_pos(i);
        planList.push_back(robot[i]);
    }
}

void plan()
{
    for (int i = 0; i < planList.size();i++) {
        Robot r = planList[i];
        int lockway = checkmap(r.m_xpos, r.m_ypos, r.m_xtarget, r.m_ytarget, r.m_id);
        if (lockway != 0) {
            lockmap(r.m_xpos, r.m_ypos, r.m_xtarget, r.m_ytarget, r.m_id, lockway);
            r.m_runway = lockway;
            removeID.push_back(i);
            runningList.push_back(r);
        }
    }
    // 清理 planList
    auto p = planList.begin();
    for (int i = removeID.size() - 1; i >= 0 ; i--) {
        planList.erase(p + removeID[i]);
    }
    removeID.clear();
    printf("After Planning\n");
}

void run()
{
    for (int i = 0; i < runningList.size(); i++) {
        Robot r = runningList[i];
        unlockmap(r.m_xpos, r.m_ypos, r.m_xtarget, r.m_ytarget, r.m_id, r.m_runway);
        robot[r.m_id - 1].m_xpos = r.m_xtarget;
        robot[r.m_id - 1].m_ypos = r.m_ytarget;
        // 领新的任务
        set_random_target_pos(r.m_id - 1);
        planList.push_back(robot[r.m_id - 1]);
    }
    runningList.clear();
    printf("After Running\n");
}


int main()
{
    init();
    printmap();
    printtask();
    for (int i = 0; i < 50; i++) {
        plan();
        printmap();
        run();
        printmap();
        printtask();
    }
}
