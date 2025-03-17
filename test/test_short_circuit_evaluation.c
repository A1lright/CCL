int counter = 0;

int check() {
    counter++;
    return 1;
}

int main() {
    if (0 && check()) {}  // 不执行check
    printf("%d", counter); // 输出0
    
    if (1 || check()) {}  // 不执行check
    printf("%d", counter); // 仍为0
    return 0;
}