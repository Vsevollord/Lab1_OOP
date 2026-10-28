#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>

class Complex {
private:
	double re;
	double im;
public:
	Complex() : re(0), im(0) {}

	Complex(double a) : re(a), im(0) {}

	Complex(double a, double b) : re(a), im(b) {}

	double get_re() const { return re; }
	double get_im() const { return im; }

	Complex operator + (const Complex& other) const {
		Complex res;
		res.re = re + other.re;
		res.im = im + other.im;
		return res;
	}

	Complex operator - (const Complex& other) const {
		Complex res;
		res.re = re - other.re;
		res.im = im - other.im;
		return res;
	}

	Complex operator * (const Complex& other) const {
		Complex res;
		res.re = re * other.re - im * other.im;
		res.im = re * other.im + im * other.re;
		return res;
	}

	Complex operator / (const Complex& other) const {
		if (other.re == 0 && other.im == 0) {
			throw std::invalid_argument("Division by zero");
		}
		Complex res;
		res.re = (re * other.re + im * other.im) / (other.re * other.re + other.im * other.im);
		res.im = (im * other.re - re * other.im) / (other.re * other.re + other.im * other.im);
		return res;
	}

	bool operator == (const Complex& other) const {
		bool res = 0;
		if (re == other.re && im == other.im) { res = 1; }
		return res;

	}

	bool operator != (const Complex& other) const {
		bool res = 1;
		if (re == other.re && im == other.im) { res = 0; }
		return res;
	}
};

std::ostream& operator<<(std::ostream& os, const Complex& c) {
	os << c.get_re();
	if (c.get_im() != 0) {
		if (c.get_im() > 0) os << "+";
		os << c.get_im() << "i";
	}
	return os;
}

class Equation {
public:
	Equation() {}
	virtual std::vector<Complex> calculate() = 0;
	virtual Complex operator ()(const Complex& x) const = 0;
	virtual void print(std::ostream& os) = 0;
	virtual ~Equation() {}
};

std::ostream& operator<<(std::ostream& os, Equation& eq) {
	eq.print(os);
	return os;
}

class LinearEquation : public Equation {
private:
	double a;
	double b;
public:
	LinearEquation(double A, double B) : a(A), b(B) {
		if (A == 0) {
			throw std::invalid_argument("linear equation: a must not be 0");
		}
	}
	std::vector<Complex> calculate() {
		return { Complex(-b / a) };
	}

	Complex operator () (const Complex& x) const {
		return Complex(a) * x + Complex(b);
	}
	void print(std::ostream& os) {
		os << a << "x";
		if (b > 0)      os << " + " << b;
		else if (b < 0) os << " - " << -b;
		os << " = 0";
	}
};

class SquareEquation : public Equation {
private:
	double a;
	double b;
	double c;

public:
	SquareEquation(double A, double B, double C) : a(A), b(B), c(C) {
		if (A == 0) {
			throw std::invalid_argument("square equation: a must not be 0");
		}
	}
	std::vector<Complex> calculate() {
		double d = -4 * a * c + b * b;
		if (d == 0) {
			Complex x1(-b / (2 * a));
			return { x1 };
		}
		if (d < 0) {
			Complex x1((-b) / (2 * a), std::sqrt(-d) / (2 * a));
			Complex x2((-b) / (2 * a), -std::sqrt(-d) / (2 * a));
			return { x1,x2 };
		}
		Complex x1((-b + std::sqrt(d)) / (2 * a));
		Complex x2((-b - std::sqrt(d)) / (2 * a));
		return { x1,x2 };

	}

	Complex operator () (const Complex& x) const {
		return Complex(a) * x * x + Complex(b) * x + Complex(c);
	}

	void print(std::ostream& os) {
		os << a << "x^2";
		if (b > 0)      os << " + " << b << "x";
		else if (b < 0) os << " - " << -b << "x";
		if (c > 0)      os << " + " << c;
		else if (c < 0) os << " - " << -c;
		os << " = 0";
	}

};

class EquationSet {
private:
	std::vector<Equation*> equations;

public:
	void add(Equation* eq) {
		if (eq == nullptr) {
			throw std::invalid_argument("equation is null");
		}
		equations.push_back(eq);
	}
	std::vector<std::vector<Complex>> calculateAll() {
		std::vector < std::vector<Complex>> res;
		for (int i = 0; i < equations.size(); i++) {
			res.push_back(equations[i]->calculate());
		}
		return res;
	}
	std::vector<Complex> calculateInPoint(const Complex& x) const {
		std::vector<Complex> res;
		for (int i = 0; i < equations.size(); i++) {
			res.push_back((*equations[i])(x));
		}
		return res;
	}
	static bool isZero(const Complex& z, double eps = 1e-9) {
		return std::abs(z.get_re()) < eps && std::abs(z.get_im()) < eps;
	}
	std::vector<Equation*> haveRoot(const Complex& x) const {
		std::vector<Equation*> res;
		for (int i = 0; i < equations.size(); i++) {
			if (isZero((*equations[i])(x))) {
				res.push_back(equations[i]);
			}

		}
		return res;
	}

	int size() const {
		return equations.size();
	}

	void printEquation(int num) {
		std::cout << *equations[num];
	}


};



class Tester {
private:
	int passed = 0;
	int failed = 0;
public:

	void check(bool condition, const std::string& name) {
		if (condition) {
			passed++;
			std::cout << "OK:" << name << "\n";
		}
		else {
			failed++;
			std::cout << "Failed:" << name << "\n";
		}
	}
	void printResults() {
		std::cout << "Failed:" << failed << " | Passed:" << passed << " of " << failed + passed << " tests" << "\n";
	}
};

void testComplex(Tester& test) {
	Complex a(1, 2);
	Complex b(3, 4);

	test.check(a + b == Complex(4, 6), "complex: +");
	test.check(a - b == Complex(-2, -2), "complex: -");
	test.check(a * b == Complex(-5, 10), "complex: *");
	test.check(a / b == Complex(0.44, 0.08), "complex: /");

	test.check(a == Complex(1, 2), "complex: ==");
	test.check(a != b, "complex: !=");

	test.check(Complex() == Complex(0, 0), "complex: default is 0");
	test.check(Complex(5) == Complex(5, 0), "complex: from double");

	Complex sum = a + b;

	test.check(a == Complex(1, 2) && b == Complex(3, 4) && sum == Complex(4, 6), "complex: operands not change");

	bool thrown = false;
	try {
		a / Complex(0, 0);
	}
	catch (const std::invalid_argument&) {
		thrown = true;

	}
	test.check(thrown, "complex: division by zero throws");

}

void testLinear(Tester& test) {
	LinearEquation equation1(2, -4);
	std::vector<Complex> roots = equation1.calculate();

	test.check(roots.size() == 1 && roots[0] == Complex(2), "linear: 2x - 4 = 0 -> x = 2");
	test.check(equation1(Complex(3)) == Complex(2), "linear: f(3) = 2");
	test.check(equation1(Complex(0, 1)) == Complex(-4, 2), "linear: f(i) = -4 + 2i");
	test.check(equation1(roots[0]) == Complex(0), "linear: f(root) = 0");

	LinearEquation equation2(5, 0);
	test.check(equation2.calculate()[0] == Complex(0), "linear: 5x = 0 -> x = 0");

	bool thrown = false;
	try {
		LinearEquation equation3(0, 5);
	}
	catch (const std::invalid_argument&) {
		thrown = true;
	}
	test.check(thrown, "linear: a = 0 throws");
}

void testSquare(Tester& test) {
	SquareEquation equation1(1, -3, 2);
	std::vector<Complex> roots1 = equation1.calculate();

	test.check(roots1.size() == 2 && roots1[0] == Complex(2) && roots1[1] == Complex(1), "square: D > 0 -> x = 2, x = 1");
	test.check(equation1(Complex(3)) == Complex(2), "square: f(3) = 2");

	SquareEquation equation2(1, -2, 1);
	std::vector<Complex> roots2 = equation2.calculate();
	test.check(roots2.size() == 1 && roots2[0] == Complex(1), "square: D = 0 -> x = 1");

	SquareEquation equation3(1, 2, 5);
	std::vector<Complex> roots3 = equation3.calculate();
	test.check(roots3.size() == 2 && roots3[0] == Complex(-1, 2) && roots3[1] == Complex(-1, -2), "square: D < 0 -> x = -1 + 2i, x = -1 - 2i");
	test.check(equation3(roots3[0]) == Complex(0) && equation3(roots3[1]) == Complex(0), "square: f(complex root) = 0");

	bool thrown = false;
	try {
		SquareEquation equation3(0, 2, 1);
	}
	catch (const std::invalid_argument&) {
		thrown = true;
	}
	test.check(thrown, "square: a = 0 throws");
}

void testEquationSet(Tester& test) {
	EquationSet empty;
	test.check(empty.size() == 0 && empty.calculateAll().empty(), "set: empty set");

	LinearEquation linear(2, -4);
	SquareEquation square1(1, -3, 2);
	SquareEquation square2(1, 2, 5);
	SquareEquation square3(1, 0, -2);

	EquationSet set;
	set.add(&linear);
	set.add(&square1);
	set.add(&square2);
	set.add(&square3);
	test.check(set.size() == 4, "set: add");

	std::vector<std::vector<Complex>> all = set.calculateAll();
	test.check(all.size() == 4 && all[0].size() == 1 && all[1].size() == 2 && all[2].size() == 2 && all[3].size() == 2, "set: solve all");
	test.check(all[0][0] == Complex(2) && all[2][0] == Complex(-1, 2), "set: solve all gives right roots");

	std::vector<Complex> values = set.calculateInPoint(Complex(3));
	test.check(values.size() == 4 && values[0] == Complex(2) && values[1] == Complex(2) && values[2] == Complex(20) && values[3] == Complex(7), "set: values at x = 3");

	std::vector<Equation*> found = set.haveRoot(Complex(2));
	test.check(found.size() == 2 && found[0] == &linear && found[1] == &square1, "set: root 2");

	found = set.haveRoot(Complex(-1, 2));
	test.check(found.size() == 1 && found[0] == &square2, "set: root -1 + 2i");

	found = set.haveRoot(Complex(std::sqrt(2.0)));
	test.check(found.size() == 1 && found[0] == &square3, "set: root sqrt(2)");

	found = set.haveRoot(Complex(100));
	test.check(found.empty(), "set: no equations with root 100");

	bool thrown = false;
	try {
		set.add(nullptr);
	}
	catch (const std::invalid_argument&) {
		thrown = true;
	}
	test.check(thrown && set.size() == 4, "set: adding nullptr throws");
}

int main() {
	Tester test;

	testComplex(test);
	testLinear(test);
	testSquare(test);
	testEquationSet(test);
	test.printResults();

	std::cout << "\nExample:\n";
	LinearEquation linear(2, -3);
	SquareEquation square(1, 2, 5);

	EquationSet set;
	set.add(&linear);
	set.add(&square);

	std::vector<std::vector<Complex>> all = set.calculateAll();
	for (int i = 0; i < all.size(); i++) {
		std::cout << "equation " << i + 1 << " ";
		set.printEquation(i);
		std::cout << " roots:";
		for (size_t j = 0; j < all[i].size(); j++) {
			std::cout << " " << all[i][j] << ";";
		}
		std::cout << "\n";
	}
	return 0;
}