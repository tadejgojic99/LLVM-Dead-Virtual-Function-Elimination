class Base {
public:
    virtual void virtual_foo() {}
    virtual int virtual_bar() {return 5;}
    void testFunction() {}
};

class testClass : public Base {
public:

};

int main() {
    Base ptr;
    ptr.virtual_bar();
    ptr.testFunction();
    ptr.virtual_foo();
    return 0;
}