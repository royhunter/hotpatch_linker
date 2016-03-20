!#/bin/bash



gcc -o test a.c

gcc -c b.c

echo "readelf -h test"
readelf -h test

echo "readelf -h b.o"
readelf -h b.o

echo "readelf -S test"
readelf -S test
echo "eadelf -S b.o"
readelf -S b.o

echo "readelf -s test"
readelf -s test
echo "readelf -s b.o"
readelf -s b.o

echo "readelf -r test"
readelf -r test
echo "readelf -r b.o"
readelf -r b.o
