cd ~/BeamlineProject
rm -rf build
mkdir build
cd build
cmake ..
make -j4


./scattering

root output.root



ScatteringAngle_Full->Draw();
gPad->SetLogy();

ScatteringAngle->Draw();
gPad->SetLogy();

TTree* t = (TTree*)_file0->Get("scattering");
t->Draw("theta");
t->Draw("energy");
t->Draw("theta","","");
