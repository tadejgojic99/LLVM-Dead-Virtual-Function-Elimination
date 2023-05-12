class Base {
public:
    virtual void foo() {}
    virtual void bar() {}
};

class Derived : public Base {
public:
    virtual void foo() override {}
};

int main() {
    Base* ptr = new Derived;
    ptr->bar(); // This will call Base::bar()
    return 0;
}