// Ovaj test prepoznaje da li je funkcija mrtva na osnovu njene implementacije
// ako je to prazna funkcija dolazi do njenog brisanja

// Bice obrisana virtual_foo i vritual_moo funkcija

class Base {
public:
  virtual void virtual_foo() {}
  virtual int virtual_bar() {
    int x = 2;
    return x + 3;
  }
  virtual void virtual_moo() {}

  // Ovu funkciju ce sam kompajler optimizovati, tj oznaci je sa "unreachable"
  virtual int koo() {}
};

class testClass : public Base {
public:
};

int main() {
  Base ptr;
  ptr.virtual_bar();
  ptr.virtual_foo();
  ptr.virtual_moo();

  return 0;
}