int main() {
    int a = 5, b = 3;
    int res = (a++ * --b) + (a > b ? 10 : 20);
    printf("%d", res); // (5 * 2)+(6>2?10:20)=10+10=20
    return 0;
}