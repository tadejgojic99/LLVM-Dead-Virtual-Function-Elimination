// Ovaj testira prepoznavanje da li je funkcija mrtva na osnovu
// toga da li se moze stici do nje preko grafa poziva

// Bice izbrisana samo virtual_moo funkcija

// virtual_bar se hvata iz main-a a virtual_foo se hvata iz virtual_bar funkcije

// Nacin na koji je pozvana funkcija preko objekta u liniji 14 olaksava
// prepoznavanje funkcije ako pozovemo preko this npr dolazi do pozivanja
// funkcije preko pokazivaca i na taj nacin poziva funkcije nismo uspeli da
// prepoznamo koja je funkcija u pitanju.

class Base {
public:
  virtual void virtual_foo() {
    int x;
    int y;
    int z;
  }
  virtual int virtual_bar() {
    Base classObj;
    classObj.virtual_foo();
    int x = 5;
    return x + 1;
  }
  virtual bool virtual_moo() {
    bool x = false;
    return x & true;
  }
};

class testClass : public Base {
public:
};

int main() {
  Base classObj;
  classObj.virtual_bar();

  return 0;
}