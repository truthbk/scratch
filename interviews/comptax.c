
#include <iostream>
#include <fstream>
#include <string>

#include <utility>
#include <unordered_map>
#include <map>
#include <multimap>

using namespace std;

//Forward Declaration
class Industry;

class Company {
    public:
        Company(string s, uint32_t cap, uint32_t rev, Industry * i = NULL) 
            : name(s)
            , industry(i);
            , market_cap(cap)
            , revenue(rev) {
                //EMPTY.
        }

        void set_name(string s) {
            name = s;
        }
        void set_industry(Industry * i) {
            industry = i;
        }
        void set_market_cap(uint32_t cap) {
            market_cap = cap;
        }
        void set_revenue(uint32_t rev) {
            revenue = rev;
        }
        string get_name() {
            return name;
        }
        Industry * get_industry(Industry * i) {
            return industry;
        }
        uint32_t get_market_cap() {
            return market_cap;
        }
        uint32_t get_revenue() {
            return revenue;
        }

    private:
        string name;
        Industry * industry;
        uint32_t market_cap;
        uint32_t revenue;
};

class Industry {
    public:
        Industry(string s, bool l=false)
            : name(s) 
            , leaf(l) 
            , parent(NULL){
                //nothing here.
            }

        bool add_subindustry(Industry ind) {
            string name(ind.get_name());
            if(subindustries.find(name) != subindustries.end()) {
                //industry exists
                return true;
            }

            if(ind.get_parent() != this) {
                //overwrite bad parent.
                ind.set_parent(this);
            }

            return subindustries.insert(make_pair(name, ind)).second;
        }

        bool add_company(Company c) {
            bool c_succ = false, rev_succ = false, cap_succ = false;
            string name(c.get_name());

            if(c.get_industry() != this) {
                //overwrite bad industry.
                c.set_industry(this);
            }
            //REFINE - what if company is already present?
            c_succ = companies.insert(make_pair(name, c)).second;
            if(!c_succ) {
                return false;
            }

            rev_succ = companies.insert(make_pair(c.get_revenue(), &companies[s])).second;
            if(!rev_succ) {
                companies.erase(name);
                return false;
            }

            cap_succ = companies.insert(make_pair(c.get_cap(), &companies[s])).second;
            if(!cap_succ) {
                multimap<uint32_t, Company *>::iterator it = comps_by_revenue.find(c.get_cap());
                while(it->second != &companies[name]) {
                    it++;
                }
                if (it != comps_by_revenue.end()) {
                    comps_by_revenue.erase(it);
                }
                companies.erase(name);
            }
            return true;
        }

        Industry * search_industry(string s) {
            Industry * ind = NULL;
            //BFS
            unordered_map<string, Industry>::const_iterator cit =
                subindustries.find(s);

            if(cit != subindustries.end()) { //HIT
                return &(cit->second);
            }

            //DFS - shouldn't make much of a difference vs BFS
            cit = subindustries.begin();
            for (; cit != subindustries.end() ; cit++) {
                ind = cit->second.search_subindustry(s);
                if(ind) {
                    return ind;
                }
            }

            return NULL;
        }

        void set_name(string s) {
            name = s;
        }
        void set_parent(Industry * industry) {
            parent = industry;
        }
        string get_name() {
            return name;
        }
        Industry * get_parent() {
            return parent;
        }

    private:
        bool leaf; //might not use
        string name;
        Industry * parent;
        unordered_map<string, Industry> subindustries;
        map<string, Company> companies;

        // if Boost available multi_index_container. Sticking to STL.
        multi_map<uint32_t, Company *>  comps_by_cap; 
        multi_map<uint32_t, Company *> comps_by_revenue;
};

class CompTaxonomy {
    public:
        CompTaxonomy() { }

        Industry * search_industry(string s) {
            Industry * ind = NULL;
            //BFS
            unordered_map<string, Industry>::const_iterator cit =
                industries.find(s);

            if(cit != industries.end()) { //HIT
                return &(cit->second);
            }

            //DFS - shouldn't make much of a difference vs BFS
            cit = industries.begin();
            for (; cit != industries.end() ; cit++) {
                ind = cit->second.search_industry(s);
                if(ind) {
                    return ind;
                }
            }

            return NULL;
        }

        bool parse_industry(vector<string> v) {
            bool status;

            //IndustryName|SomeIndustry|ContainedInSomeOther|
            if(v[0]!=industry_pre) {
                return false;
            }

            string name(v[1]);
            string contained_in ="";
            if(v.size()>2) {
                contained_in = v[2];
            }

            Industry i(name);
            if(contained_in.empty()) {
                return industries.insert(make_pair(name, i)).second;
            } else {
                Industry *parent = search_industry(contained_in);
                if(!parent) {
                    return false;
                }

                status = parent->add_subindustry(i);
            }

            return status;

        }
        bool parse_company(vector<string> &v) {
            bool status;
            uint32_t cap, rev;

            //Company|CompanyName|Industry|MarketCap|IndustryRevenue|
            if(v[0]!=company_pre || v.size() != 5) {
                return false;
            }

            string name(v[1]);
            string industry(v[2]);

            stringstream conv;
            conv << v[3];
            if(!(conv >> cap)) {
                return false;
            }

            conv << v[4];
            if(!(conv >> rev)) {
                return false;
            }

            Industry * i = search_industry(industry);
            if(!i) {
                return false;
            }

            Company c(name, cap, rev) 
            status = i->add_company(c);

            return status;
        }

        bool parse_input(string f) {
            string line;
            ifstream ifile(f);
            vector<string> split;

            if(ifile.is_open()) {
                while( getline(ifile, line)) {
                    split = blast(line, token);

                    if(v[0] == industry_pre) {
                        parse_industry(v);
                    } else if(v[0] == company_pre) {
                        parse_company(v);
                    } else {
                        //nothing(comment?).
                    }
                }
                ifile.close();
            } else {
            return false;
            }

            return true;
        }
    private:
        vector<string> blast(string s, char tok) {
            vector<string> v;
            int index = 0;

            string stok(s);
            while(stok.length()) {
                index = stok.find(token);
                if(index == string::npos) {
                    v.push_back(stok);
                    stok = "";
                }
                if(index == 0) {
                    stok = stok.substr(1); //skip token
                } else {
                    v.push_back(stok.substr(0, index-1));
                    stok = stok.substr(index);

                }
            }
            return v;
        }

        unordered_map<string, Industry> industries;
        const char token = '|';
        const string comment_pre = "//";
        const string industry_pre = "IndustryName";
        const string company_pre  = "Company";
};
