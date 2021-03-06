#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <string>
#include <cstdarg>
#include <stdexcept>
#include "generalutil.hpp"

const int maxIterations = 1000; //The maximum iterations executed for some functions. Increasing the value gives higher accuracy but increases load time.

/*
 * MaxIterations currently affects:
 * - polyDivide
 */

const int maxPower = 100; //Specifies the maximum power allowed. Used by aggregate().

class Term;
class Polynomial;

/*
 * class Term:
 *
 * A term in a polynomial. Used to define polynomials.
 *
 * Private:
 * double coefficient: the coefficient of the term.
 * int power: the power of x of that term.
 *
 * Public:
 * double getCoefficient(): returns coefficient.
 * int get_power(): returns power.
 * void setCoefficient(double value): sets the coefficient to value.
 * void setPower(int value): set the power to value.
 * Polynomial toPolynomial(): converts the Term to a Polynomial. Defined later. Currently not a lot of use.
 *
 * Static functions:
 * Term termMultiply(Term x, Term y): returns the product of x and y. Used for polynomial multiplication.
 */

class Term
{
    private:
        double coefficient;
        int power; //Power of x
    public:
        Term()
        {
        }
        Term(double v, int p)
        {
            coefficient = v;
            power = p;
        }
        double getCoefficient()
        {
            return coefficient;
        }
        int get_power()
        {
            return power;
        }
        void setCoefficient(double value)
        {
            coefficient = value;
        }
        void setPower(double value)
        {
            power = value;
        }
        Polynomial toPolynomial();
        Term operator*(Term y)
        {
            return Term(this -> getCoefficient() * y.getCoefficient(), this -> get_power() + y.get_power());
        }
};

Term termMultiply(Term x, Term y) //Serves as a utility function for polyMultiply.
{
    return Term(x.getCoefficient() * y.getCoefficient(), x.get_power() + y.get_power());
}

/*
 * class Polynomial:
 *
 * Private:
 * std::vector<Terms> terms: contains a vector of all terms of that Polynomial.
 *
 * Public
 * void sort_terms(int low, int high): sort the terms in descending order of power. low and high determine the starting index and the ending index respectively. Uses quicksort algorithm. https://www.geeksforgeeks.org/quick-sort/
 * int partition(int low, int high): purely a utility function for sort_terms.
 * void swap_terms(Term *a, Term *b): a utility function for sort_terms.
 * void aggregate(): aggregates like terms and deletes any zero terms. Algorithm needs improvement (as of now)
 * void init(): a helper function that runs sort_terms and aggregate.
 * int search_for_power(int l, int r, int target): uses binary search algorithm (https://www.geeksforgeeks.org/binary-search/) to search for the index of a specific power. Useful for other functions.
 * double get_coefficient_of_power(int power): gets the coefficient of a certain power of x (using search_for_power). Returns 0 if not found (the coefficient is mathematically 0. NOT -1).
 * int get_power(): gets the maximum power of the Polynomial and returns it as an integer.
 * int count_terms(): count the number of terms a Polynomial has.
 * Term get_term(int index): returns terms[index].
 * void add_term(Term t): adds t to terms.
 * std::string to_string(): converts the Polynomial to a string.
 * double substitute(double value): substitutes value into x, the returns the value of the resulting polynomial.
 * Polynomial operator+(Polynomial Q): overloads the + operator. Same as polyAdd.
 * Polynomial operator-(Polynomial Q): overloads the - operator. Same as polySubtract.
 * Polynomial operator*(Polynomial Q): overloads the * operator. Same as polyMultiply.
 * Polynomial operator/(Polynomial Q): overloads the / operator. Same as polyDivide.
 * Polynomial operator%(Polynomial Q): overloads the % operator. Same as polyMod.
 *
 * Static functions:
 *
 * Polynomial arithmetics (deprecated, now replaced by overloading default operators):
 *
 * Polynomial polyAdd(Polynomial P, Polynomial Q): returns the sum of P and Q.
 * Polynomial polyMultiply(Polynomial P, Polynomial Q): returns the product of P and Q.
 * Polynomial polySubtract(Polynomial P, Polynomial Q): subtracts Q from P (returns P - Q).
 * Polynomial polyDivide(Polynomial P, Polynomial Q): returns P/Q (without remainder). Hardcoded to loop 1000 times max to avoid infinite loops caused by precision errors.
 * Polynomial polyMod(Polynomial P, Polynomial Q): returns the remainder of P/Q.
 *
 * Alternate constructors:
 *
 * Polynomial poly_parse_string(std::string str): returns a polynomial from a string (buggy as of now)
 * Polynomial poly_parse_array(double array[], int length): returns a polynomial from an array.
 * Polynomial poly_parse_vector(std::vector<double>): returns a polynomial from a vector.
 * Polynomial input_polynomial(std::string name): a user-friendly function to create a new polynomial. 'name' refers to the name of the polynomial referenced in the guiding messages.
 *
 */

class Polynomial
{
    protected:
        std::vector<Term> terms; //A vector of terms. For example, 3x^2 + 2x would become {{3, 2}, {2, 1}}.
    public:
        Polynomial(std::vector<Term> t)
        {
            terms = t;
        }
        Polynomial(double c) //Defining using a constant
        {
            Term t (c, 0);
            terms.push_back(t);
        }
        /*Polynomial(int count, ...)
        {
            terms.clear();
            std::va_list args;
            va_start(args, count);
            int i;
            for(i = count - 1; i >= 0; i--)
            {
                Term temp (va_arg(args, double), i);
                terms.push_back(temp);
            }
        }*/
        Polynomial()
        {
            terms.clear();
        }
        void sort_terms(int low, int high) //Sort the terms using the quicksort algorithm. https://www.geeksforgeeks.org/quick-sort/
        {
            if(low < high)
            {
                int pi = partition(low, high);
                sort_terms(low, pi - 1);
                sort_terms(pi + 1, high);
            }
        }
        int partition(int low, int high) //A utility for sort_terms()
        {
            int pivot = terms[high].get_power();
            int i = low - 1;
            int j;
            for(j = low; j <= high - 1; j++)
            {
                if(terms[j].get_power() < pivot)
                {
                    i++;
                    swap_terms(&terms[i], &terms[j]);
                }
            }
            swap_terms(&terms[i + 1], &terms[high]);
            return (i + 1);
        }
        void swap_terms(Term *a, Term *b) //A utility for sort_terms()
        {
            Term t = *a;
            *a = *b;
            *b = t;
        }
        void aggregate() //Aggregate like terms. Algorithm needs improvement but at least works.
        {
            double coef[maxPower] = {0};
            int i;
            for(i = 0; i < terms.size(); i++)
            {
                coef[terms[i].get_power()] += terms[i].getCoefficient();
            }
            terms.clear();
            for(i = maxPower - 1; i >= 0; i--)
            {
                if(coef[i] != 0)
                {
                    Term temp (coef[i], i);
                    terms.push_back(temp);
                }
            }
        }
        void init() //Initialises the polynomial
        {
            sort_terms(0, terms.size() - 1);
            std::reverse(terms.begin(), terms.end());
            aggregate();
        }
        int search_for_power(int l, int r, int target) //Search for the index of a power.
        {
            if(r >= l)
            {
                int mid = l + (r - l) / 2;
                if(terms[mid].get_power() == target)
                {
                    return mid;
                }
                if(terms[mid].get_power() < target)
                {
                    return search_for_power(l, mid - 1, target);
                }
                return search_for_power(mid + 1, r, target);
            }
            return -1;
        }
        double get_coefficient_of_power(int power)
        {
            init();
            int p = search_for_power(0, terms.size() - 1, power);
            if(p == -1)
            {
                return 0;
            }
            else
            {
                return terms[p].getCoefficient();
            }
        }
        int get_power()
        {
            init();
            return terms[0].get_power();
        }
        int count_terms()
        {
            init();
            return terms.size();
        }
        Term get_term(int index)
        {
            return terms[index];
        }
        void add_term(Term t)
        {
            terms.push_back(t);
        }
        std::string to_string()
        {
            init();
            std::string str = "";
            int i;
            for(i = get_power(); i >= 0; i--)
            {
                str.append(my::to_string(get_coefficient_of_power(i)));
                if(i >= 2)
                {
                    str.append(" x^");
                    str.append(std::to_string(i));
                    str.append(" + ");
                }
                else if(i == 1)
                {
                    str.append(" x ");
                    str.append(" + ");
                }
            }
            return str;
        }
        double substitute(double value)
        {
            double result = 0;
            int i;
            for(i = 0; i < terms.size(); i++)
            {
                result += pow(value, terms[i].get_power()) * terms[i].getCoefficient();
            }
            return result;
        }
        Polynomial operator+(Polynomial Q)
        {
            Polynomial result;
            this -> init();
            Q.init();
            int i;
            for(i = 0; i < this -> count_terms(); i++)
            {
                result.add_term(this -> get_term(i));
            }
            for(i = 0; i < Q.count_terms(); i++)
            {
                result.add_term(Q.get_term(i));
            }
            result.init();
            return result;
        }
        Polynomial operator-(Polynomial Q)
        {
            Polynomial neg (-1.0);
            Polynomial R = Q * neg;
            return *this + R;
        }
        Polynomial operator*(Polynomial Q)
        {
            Polynomial result;
            this -> init();
            Q.init();
            int i, j;
            for(i = 0; i < this -> count_terms(); i++)
            {
                for(j = 0; j < Q.count_terms(); j++)
                {
                    result.add_term(this -> get_term(i) * Q.get_term(j));
                }
            }
            result.init();
            return result;
        }
        Polynomial operator/(Polynomial Q)
        {
            this -> init();
            Polynomial temp = *this;
            Polynomial result;
            double coef;
            int pow;
            Term tm;
            int i = 0;
            while(temp.get_power() >= Q.get_power() && i < maxIterations) //A set limit of iterations to stop infinite loops due to double precision errors.
            {
                coef = temp.get_coefficient_of_power(temp.get_power());
                pow = temp.get_power() - Q.get_power();
                tm.setCoefficient(coef / Q.get_coefficient_of_power(Q.get_power()));
                tm.setPower(pow);
                result.add_term(tm);
                temp = temp - tm.toPolynomial() * Q;
                temp.init();
                i++;
            }
            return result;
        }
        Polynomial operator%(Polynomial Q)
        {
            this -> init();
            Q.init();
            return *this - (Q * (*this / Q));
        }
};

Polynomial Term::toPolynomial()
{
    std::vector<Term> t;
    t.push_back(Term(coefficient, power));
    Polynomial P(t);
    return P;
}

/*
Polynomial polyMultiply(Polynomial, Polynomial);

Polynomial polyAdd(Polynomial P, Polynomial Q)
{
    Polynomial result;
    P.init();
    Q.init();
    int i;
    for(i = 0; i < P.count_terms(); i++)
    {
        result.add_term(P.get_term(i));
    }
    for(i = 0; i < Q.count_terms(); i++)
    {
        result.add_term(Q.get_term(i));
    }
    result.init();
    return result;
}

Polynomial polySubtract(Polynomial P, Polynomial Q)
{
    Polynomial neg (-1.0);
    Polynomial R = polyMultiply(Q, neg);
    return polyAdd(P, R);
}

Polynomial polyMultiply(Polynomial P, Polynomial Q)
{
    Polynomial result;
    P.init();
    Q.init();
    int i, j;
    for(i = 0; i < P.count_terms(); i++)
    {
        for(j = 0; j < Q.count_terms(); j++)
        {
            result.add_term(termMultiply(P.get_term(i), Q.get_term(j)));
        }
    }
    result.init();
    return result;
}

Polynomial polyDivide(Polynomial P, Polynomial Q)
{
    P.init();
    Polynomial temp = P;
    Polynomial result;
    double coef;
    int pow;
    Term tm;
    int i = 0;
    while(temp.get_power() >= Q.get_power() && i < maxIterations) //A set limit of iterations to stop infinite loops due to double precision errors.
    {
        coef = temp.get_coefficient_of_power(temp.get_power());
        pow = temp.get_power() - Q.get_power();
        tm.setCoefficient(coef / Q.get_coefficient_of_power(Q.get_power()));
        tm.setPower(pow);
        result.add_term(tm);
        temp = polySubtract(temp, polyMultiply(tm.toPolynomial(), Q));
        temp.init();
        i++;
    }
    return result;
}

Polynomial polyMod(Polynomial P, Polynomial Q)
{
    P.init();
    Q.init();
    return polySubtract(P, polyMultiply(Q, polyDivide(P, Q)));
}

The above are past functions that manipulates arithmetic operators with polynomials; now they are replaced by overloading operators.

*/

Polynomial poly_parse_string(std::string str) //Buggy, needs fix.
{
    std::vector<size_t> delimiterLoc;
    delimiterLoc.push_back(-1);
    size_t loc = -1;
    do
    {
        loc = str.find(" ", loc + 1);
        delimiterLoc.push_back(loc);
    }
    while (loc != std::string::npos);
    std::vector<double> list;
    for(int i = 0; i <= delimiterLoc.size(); i++)
    {
        list.push_back(std::stod(str.substr(delimiterLoc[i] + 1, delimiterLoc[i + 1] - delimiterLoc[i]), NULL));
    }
    std::vector<Term> t;
    for(int i = 0; i < list.size(); i++)
    {
        Term temp (list[i], list.size() - i - 1);
        t.push_back(temp);
    }
    Polynomial P (t);
    return P;
}

Polynomial poly_parse_array(double array[], int length) //A functional alternative to poly_parse_string.
{
    Polynomial P;
    Term term;
    for(int i = 0; i < length; i++)
    {
        term.setCoefficient(array[i]);
        term.setPower(length - i - 1);
        P.add_term(term);
    }
    return P;
}

Polynomial poly_parse_vector(std::vector<double> vector)
{
    Polynomial P;
    Term term;
    for(int i = 0; i < vector.size(); i++)
    {
        term.setCoefficient(vector[i]);
        term.setPower(vector.size() - i - 1);
        P.add_term(term);
    }
    return P;
}

Polynomial input_polynomial(std::string name) //A user-friendly function to create a polynomial.
{
    int power;
    double temp;
    std::vector<double> vec;
    BEGIN:
    std::cout << "Enter the power of the " + name + " polynomial:" << std::endl;
    std::cin >> power;
    if(power < 0) //Checks the validity of the input, to prevent errors.
    {
        std::cout << "Invalid number entered! Power should be at least 1." << std::endl;
        goto BEGIN;
    }
    std::cout << "Enter the terms of the " + name + " polynomial, from the highest power to the constant term" << std::endl;
    for(int i = 0; i < power + 1; i++) //+1 due to the constant term (i.e. term without x)
    {
        std::cin >> temp;
        vec.push_back(temp);
    }
    Polynomial P = poly_parse_vector(vec);
    return P;
}

class Quadratic : Polynomial
{
    public:
        Quadratic(std::vector<Term> terms)
        {
            this -> terms = terms;
        }
        Quadratic(double a, double b, double c)
        {
            if(a == 0)
            {
                throw std::invalid_argument("x^2 coefficient must not be 0.");
            }

        }
};

class Matrix;
