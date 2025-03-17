int main() {
    int a = 0;
    {
        int a = 1;  // 合法作用域嵌套
        printf("%d", a); // 输出1
    }
    printf("%d", a); // 输出0
    return 0;
}