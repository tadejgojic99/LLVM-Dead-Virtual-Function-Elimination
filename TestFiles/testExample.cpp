class Base {
public:
    virtual void virtual_foo() {}
    virtual void virtual_bar() {}
    void testFunction() {}
};

class testClass : public Base {
public:

};

int main() {
    Base* ptr = new testClass();
    ptr->virtual_bar();
    ptr->testFunction();
    return 0;
}