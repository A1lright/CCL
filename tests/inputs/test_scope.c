int a = 10;               // 全局变量
int main() {
    int a = 20;           // 局部变量覆盖全局
    {
        int a = 30;       // 内层作用域再次覆盖
        printf("%d", a);  // 输出 30
    }
    printf("%d", a);      // 输出 20
    return 0;
}