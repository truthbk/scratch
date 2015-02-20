//
// The code below should allow us to identify a majority element in a vector arr,
// where that element (el) is present more than vector arr.size()/2 times.
// We should also identify this element if it's not there.
//
// It has two very important constraints: computational complexity should be 
// O(kN) = O(N) and space complexity should be O(m) (constant).
//
// Based on a Radix sort approach: we gradually build a mask knowing that more than
// half of the elements should match a particular mask. And we gradually iterate to
// make this mask match the actual value.
//
//UNTESTED and never compiled!
int process_majority(std::vector<int>& arr, int& v) {
    int mask = 0;
    int one = 0,zero = 0;

    for(int i=0 ; i<sizeof(int)*8 ; i++) {
        int mask_match = 0;
        mask |= 1<<i;
        for(j=0 ; j<arr.size() ; j++) {
            if(arr[j] & mask) {
                one++;
                if(arr[j] == mask) {
                    if(++mask_match > arr.size()/2) {
                        v = mask;
                        return 0;
                    }
                }
            } else {
                zero++;
            }
        }
        if(zero > arr.size()/2) {
            //switch off bit on mask
            mask ^= 1<<i;
        } else if (zero == one) {
            return -1;
        }
    }
    return -1;
}
