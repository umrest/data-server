
    class BitArray8
    {
        public:
        char aByte = 0;
       BitArray8()
        {
            
        }

        void SetBit(int pos, bool value)
        {
            if (value)
            {
                //left-shift 1, then bitwise OR
                aByte = (aByte | (1 << pos));
            }
            else
            {
                //left-shift 1, then take complement, then bitwise AND
                aByte = (aByte & ~(1 << pos));
            }
        }

        bool GetBit(int pos)
        {
            //left-shift 1, then bitwise AND, then check for non-zero
            return ((aByte & (1 << pos)) != 0);
        }
    };