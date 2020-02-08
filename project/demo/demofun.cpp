#include <functional>

int fun1(int v)
{
    return printf("v=%d\n", v);
}

struct FUN_T
{
    std::function<int(int)>& fun;
};

void test(std::function<int(int)>& fun)
{
    FUN_T stV = {fun};

    //stV.fun.swap(fun);
    printf("%p, %p\n", &stV.fun, &fun);
    stV.fun(100);
    fun(200);
    if (&stV.fun == &fun)
        printf("true\n");
    else
        printf("false\n");
}

int main()
{
    std::function<int(int)> obj = fun1;
    test(obj);
    obj(300);
    printf("%p\n", &obj);
    return 0;
}
