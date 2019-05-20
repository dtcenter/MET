for f in *.h
do
    echo "____"
    echo $f
    diff $f /Users/fillmore/MET/met_feature_1085_tcrmw/met/src/basic/vx_util/$f
done

for f in *.cc
do
    echo "____"
    echo $f
    diff $f /Users/fillmore/MET/met_feature_1085_tcrmw/met/src/basic/vx_util/$f
done
