// Vrsi test da gde se prepoznaje da je f-ja mrtva zato sto vraca konstantu 
// virtual_bar bi trebale biti izbrisana zato sto vraca konstantu vrednost

// Funkcije virtual_bar i virtual_moo vracaju const vrednosti pa se brisu

class testClass {
public:
    virtual int virtual_foo() {int x; 
                                x = 50;
                                return x/2;
                               }
    virtual int virtual_bar() {return 5;}
    virtual bool virtual_moo() {return true;}
};

int main() {
    testClass classObj;
    classObj.virtual_bar();
    classObj.virtual_foo();
    classObj.virtual_moo();
    return 0;
}