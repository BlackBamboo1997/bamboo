#include "TcpServer.h"

#include "TcpConnection.h"
#include "ThreadPool.h"
#include "Logging.h"
#include "Thread.h"
#include "EventLoop.h"
#include "InetAddress.h"

#include <vector>
#include <utility>
#include <iostream>

#include <stdio.h>
#include <unistd.h>

using namespace bamboo;
using namespace bamboo::net;
using std::vector;
using std::cin;
using std::cout;

class SudokuServer
{
public:
    SudokuServer(EventLoop *loop, InetAddress listen)
        : loop_(loop),
          server_(loop, listen, "SudokuServer"),
          workPool_()
    {
        server_.setMessageCallback(std::bind(&SudokuServer::OnMessage, this, _1, _2, _3));
        server_.setThreadNum(0);
        workPool_.setThreadNum(3);
    }

    void start()
    {
        server_.start();
        workPool_.start();
    }
    void resolveSudoku(const TcpConnectionPtr &conn, const string &msg, Timestamp recivetime);

private:
    void OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        string msg(buf->retrieveAllAsString());
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
        conn->send(string("solving\n"));
        workPool_.push(std::bind(&SudokuServer::resolveSudoku, this, conn, msg, time));
    }

    std::vector<std::vector<char>> parseString(const string &msg);
    bool isVaild(vector<vector<char>> &sudoku,
                 vector<vector<bool>> &rows,
                 vector<vector<bool>> &cols,
                 vector<vector<vector<bool>>> &blocks,
                 vector<std::pair<int, int>> &needSolve);
    bool dfs(std::vector<std::vector<char>> &sudoku,
             int index,
             vector<vector<bool>> &rows,
             vector<vector<bool>> &cols,
             vector<vector<vector<bool>>> &blocks,
             vector<std::pair<int, int>> &needSolve);
    string reponse(vector<vector<char>> &sudoku);

    EventLoop *loop_;
    TcpServer server_;
    ThreadPool workPool_;
};

vector<vector<char>> SudokuServer::parseString(const string &str)
{
    vector<vector<char>> result(9, vector<char>(9));
    for (int i = 0; i < 81; ++i)
    {
        result[i / 9][i % 9] = str[i];
    }
    return result;
}

bool SudokuServer::isVaild(vector<vector<char>> &sudoku,
                           vector<vector<bool>> &rows,
                           vector<vector<bool>> &cols,
                           vector<vector<vector<bool>>> &blocks,
                           vector<std::pair<int, int>> &needSolve)
{
    for (int i = 0; i < 9; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            int pos = sudoku[i][j] - '0' - 1;
            if (pos >= 0 && pos < 9)
            {
                if (!rows[i][pos] && !cols[j][pos] && !blocks[i / 3][j % 3][pos])
                {
                    rows[i][pos] = cols[j][pos] = blocks[i / 3][j % 3][pos] = 1;
                }
                else
                    return false;
            }
            else
            {
                needSolve.push_back(std::make_pair(i, j));
            }
        }
    }
    return true;
}

bool SudokuServer::dfs(vector<vector<char>> &sudoku,
                       int index,
                       vector<vector<bool>> &rows,
                       vector<vector<bool>> &cols,
                       vector<vector<vector<bool>>> &blocks,
                       vector<std::pair<int, int>> &needSolve)
{
    if (index == needSolve.size())
        return true;
    int row = needSolve[index].first;
    int col = needSolve[index].second;
    // printf("row = %d, col = %d, current index %d\n", row, col, index);
    for (int i = 0; i < 9; ++i)
    {
        if (!rows[row][i] && !cols[col][i] && !blocks[row / 3][col % 3][i])
        {
            rows[row][i] = cols[col][i] = blocks[row / 3][col % 3][i] = 1;
            sudoku[row][col] = '1' + i;
            if (dfs(sudoku, index + 1, rows, cols, blocks, needSolve))
                return true;
            sudoku[row][col] = ',';
            rows[row][i] = cols[col][i] = blocks[row / 3][col % 3][i] = 0;
        }
    }
    return false;
}

void SudokuServer::resolveSudoku(const TcpConnectionPtr &conn, const string &msg, Timestamp recivetime)
{
    vector<vector<bool>> rows(9, vector<bool>(9, false));
    vector<vector<bool>> cols(9, vector<bool>(9, false));
    vector<vector<vector<bool>>> blocks(3, vector<vector<bool>>(3, vector<bool>(9, false)));
    vector<std::pair<int, int>> needSolve;

    vector<vector<char>> sudoku = parseString(msg);

    //there is work thread, we need use shared_ptr, avoid data is destroyed
    std::shared_ptr<string> result(new string);
    if (isVaild(sudoku, rows, cols, blocks, needSolve))
    {
        if (dfs(sudoku, 0, rows, cols, blocks, needSolve))
        {
            result->append(reponse(sudoku));
            Timestamp now(Timestamp::now());
            double seconds = timeDifference(now, recivetime);
            result->append("time use: " + std::to_string(seconds) + " seconds\n");
            // EventLoop* loop = conn->getLoop();
            // loop->queueInLoop(std::bind<void (TcpConnection::*)(const string&)>(&TcpConnection::send, conn, ref(result)));
            // std::shared_ptr<string> res(new string(result));
        }
        else
        {
            result->append("no solve\n");
        }
    }
    else
    {
        result->append("no vaild\n");
    }
    conn->send(result);
}

string SudokuServer::reponse(vector<vector<char>> &sudoku)
{
    string result;
    for (auto &vec : sudoku)
    {
        for (auto &c : vec)
        {
            result.push_back(c);
            result.push_back(' ');
        }
        result.push_back('\n');
    }
    return result;
}

int main()
{
    // Logger::setLogLevel(Logger::TRACE);
    LOG_INFO << "pid = " << getpid() << ", tid = " << currentthread::tid();
    LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
    EventLoop loop;
    InetAddress listenAddr(2000, false);
    SudokuServer server(&loop, listenAddr);

    server.start();

    loop.loop();
}