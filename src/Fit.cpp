#ifndef __CINT__
#include <RooGlobalFunc.h>
#include <RooMsgService.h>
#endif

#include "Fit.h"
#include <iostream>
#include <sstream>

#include <TTree.h>
#include <TMath.h>
#include <TMinuit.h>
#include <TFitter.h>
#include <TRandom.h>


#include <TAxis.h>
#include <TGraphErrors.h>
#include <TH2.h>

using namespace RooFit ;

Fit::Fit()
{
}

Fit::~Fit()
{
	clear() ;
}


void Fit::clear()
{
	fitMap.clear() ;
	histoMap.clear() ;
}

//may bug
void Fit::readFile(const char* file)
{
	clear() ;

	TFile* histoFile = new TFile(file, "READ") ;

	for ( unsigned int i = 1 ; i < 101 ; i++ )
	{
		std::stringstream toto ;
		toto << i << "GeV" ;
		histoMap[i] = std::shared_ptr<TH1>( dynamic_cast<TH1*>( histoFile->Get(toto.str().c_str()) ) ) ;
		histoMap[i]->SetDirectory(0) ;
	}

	std::vector<float> toErase ;

	for( auto it = histoMap.begin() ; it != histoMap.end() ; ++it)
	{
		if ( it->second == nullptr )
			toErase.push_back(it->first) ;
	}

	for(std::vector<float>::iterator it = toErase.begin() ; it != toErase.end() ; ++it)
		histoMap.erase(*it) ;

	histoFile->Close() ;
}

void Fit::loadHistos(const std::map<float,std::shared_ptr<TH1>>& _histoMap)
{
	clear() ;
	histoMap = _histoMap ;
}

void Fit::fitHisto(float energy)
{
	RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
	RooMsgService::instance().setSilentMode(true) ;

	if ( !histoMap.count(energy) )
	{
		std::cerr << "Fit::fitHisto(" << energy << ") : histo not found" << std::endl ;
		return ;
	}

//	int nEntries = static_cast<int>( histoMap[energy]->GetEntries() ) ;

	RooRealVar x("Energy" , "Energy (GeV)" , 0 , energy*2) ;
	RooDataHist dh("histo" , "GeV" , x , Import(*histoMap[energy]) ) ;

	std::stringstream frameTitle ;
	frameTitle << energy << "GeV frame" ;
//	RooPlot* frame = x.frame( Title( frameTitle.str().c_str() ) ) ;

//	dh.plotOn(frame) ;

	RooRealVar mean("mean","mean",energy,0,2*energy) ;
	RooRealVar sigma("sigma" , "sigma" , energy/10 , 0 , 2*energy/5) ;
	RooRealVar alpha("alpha" , "alpha" , 1 , 0 , 50) ;
	RooRealVar n("n" , "n" , 1 , 0 , 200) ;
	RooCBShape cb("cb" , "cb" , x , mean , sigma , alpha , n) ;
	cb.printEvalErrors(std::cout , -1) ;
	cb.fitTo(dh) ;
//	cb.plotOn(frame , Normalization(nEntries,RooAbsReal::NumEvent)) ;


	RooGaussian gx("gauss" , "gauss" , x , mean , sigma) ;

	//	RooAbsReal* icb = cb.createIntegral(x) ;
	//	double integralCB = icb->getVal() ;
	//	RooAbsReal* igx = gx.createIntegral(x) ;
	//	double integralGauss = igx->getVal() ;

	//	gx.plotOn(frame , LineColor(kBlue-2) , LineStyle(kDashed) , Normalization( nEntries*(integralGauss/integralCB), RooAbsReal::NumEvent ) ) ;

	//	//x.setRange("1.5sigma" , mean.getVal()-1.5*sigma.getVal() , mean.getVal()+1.5*sigma.getVal() ) ;
	//	// x.setRange("2sigma" , mean.getVal()-2.0*sigma.getVal() , mean.getVal()+2.0*sigma.getVal() ) ;
	//	x.setRange("rightPart" , mean.getVal()-2.0*sigma.getVal() , energy*2 ) ;
	//	RooRealVar meanG("meanG" , "meanG" , mean.getVal() , 0 , 2*energy) ;
	//	RooRealVar sigmaG("sigmaG" , "sigmaG" , sigma.getVal() , 0 , 2*energy/5) ;
	//	RooGaussian gauss("gauss" , "gauss" , x , meanG , sigmaG) ;
	//	//gauss.fitTo(dh, Range("1.5sigma")) ;
	//	// gauss.fitTo(dh, Range("2sigma")) ;
	//	gauss.fitTo(dh, Range("rightPart")) ;
	//	gauss.plotOn(frame , LineColor(kRed) ) ;
	//	gauss.plotOn(frame, Range("Full") , LineStyle(kDashed) , LineColor(kRed) ) ;
	//	gauss.paramOn(frame, Layout(0.10,0.45,0.90)) ;
	//	frame->getAttText()->SetTextSize(0.03f) ;

	//frame->Draw() ;
	/*
	cout << "         " << "  mean " << " | " << " sigma " << " | " << " resol " << endl ;
	cout << "    CB : " << mean.getVal() << " | " << sigma.getVal() << " | " << 100*sigma.getVal()/mean.getVal() << endl ;
	cout << " GAUSS : " << meanG.getVal() << " | " << sigmaG.getVal() << " | " << 100*sigmaG.getVal()/meanG.getVal() << endl ;
	cout << endl ;
	*/

	//	double meanMean = ( mean.getVal()+meanG.getVal() )/ 2.0 ;
	//	double meanResol = ( 100*( sigma.getVal()/mean.getVal() + sigmaG.getVal()/meanG.getVal() ) ) / 2.0 ;
	//	double errMean = abs( ( mean.getVal()-meanG.getVal() )/ 2.0 ) ;
	//	double errResol = abs( ( 100*( sigma.getVal()/mean.getVal() - sigmaG.getVal()/meanG.getVal() ) ) / 2.0 ) ;

	/*
	cout << "MEAN : " << meanMean << " +- " << errMean << endl ;
	cout << "RESOL : " << meanResol << " +- " << errResol << endl ;
	cout << endl ;
	cout << "LIMIT : " << mean.getVal() - alpha.getVal()*sigma.getVal() << endl ;
	cout << "    % : " << 100*( integralGauss/integralCB ) << endl ;
	*/

	//	fitMap[energy].mean = meanMean ;
	//	fitMap[energy].resol = meanResol ;
	//	fitMap[energy].meanErr = errMean ;
	//	fitMap[energy].resolErr = errResol ;

	fitMap[energy].mean = mean.getVal() ;
	fitMap[energy].resol = sigma.getVal()/mean.getVal() ;
	fitMap[energy].meanErr = mean.getError() ;
	fitMap[energy].resolErr = std::sqrt( (sigma.getVal()*mean.getError()/(mean.getVal()*mean.getVal()))*(sigma.getVal()*mean.getError()/(mean.getVal()*mean.getVal()))  +  (sigma.getError()/mean.getVal())*(sigma.getError()/mean.getVal())  ) ;

	fitMap[energy].sigma = sigma.getVal() ;
	fitMap[energy].alpha = alpha.getVal() ;
	fitMap[energy].n = n.getVal() ;

//	plotMap[energy] = frame ;

}

void Fit::fitAllHistos()
{
	for(auto it : histoMap)
		fitHisto(it.first) ;
}

void Fit::drawPlot(float energy)
{
	if ( !fitMap.count(energy) )
	{
		std::cerr << "Fit::drawPlot(" << energy << ") : plot not found" << std::endl ;
		return ;
	}

	fitResults fitRes = fitMap.at(energy) ;
	RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
	RooMsgService::instance().setSilentMode(true) ;

	if ( !histoMap.count(energy) )
	{
		std::cerr << "Fit::fitHisto(" << energy << ") : histo not found" << std::endl ;
		return ;
	}

	int nEntries = static_cast<int>( histoMap[energy]->GetEntries() ) ;

	RooRealVar x("Energy" , "Energy (GeV)" , 0 , energy*2) ;
	RooDataHist dh("histo" , "GeV" , x , Import(*histoMap[energy]) ) ;

	std::stringstream frameTitle ;
	frameTitle << energy << "GeV frame" ;
	RooPlot* frame = x.frame( Title( frameTitle.str().c_str() ) ) ;

	dh.plotOn(frame) ;

	RooRealVar mean("mean" , "mean" , fitRes.mean) ;
	RooRealVar sigma("sigma" , "sigma" , fitRes.sigma) ;
	RooRealVar alpha("alpha" , "alpha" , fitRes.alpha) ;
	RooRealVar n("n" , "n" , fitRes.n) ;
	RooCBShape cb("cb" , "cb" , x , mean , sigma , alpha , n) ;
	cb.plotOn(frame , Normalization(nEntries,RooAbsReal::NumEvent)) ;

	frame->Draw() ;


//	RooGaussian gx("gauss" , "gauss" , x , mean , sigma) ;

	//	RooAbsReal* icb = cb.createIntegral(x) ;
	//	double integralCB = icb->getVal() ;
	//	RooAbsReal* igx = gx.createIntegral(x) ;
	//	double integralGauss = igx->getVal() ;

	//	gx.plotOn(frame , LineColor(kBlue-2) , LineStyle(kDashed) , Normalization( nEntries*(integralGauss/integralCB), RooAbsReal::NumEvent ) ) ;

	//	//x.setRange("1.5sigma" , mean.getVal()-1.5*sigma.getVal() , mean.getVal()+1.5*sigma.getVal() ) ;
	//	// x.setRange("2sigma" , mean.getVal()-2.0*sigma.getVal() , mean.getVal()+2.0*sigma.getVal() ) ;
	//	x.setRange("rightPart" , mean.getVal()-2.0*sigma.getVal() , energy*2 ) ;
	//	RooRealVar meanG("meanG" , "meanG" , mean.getVal() , 0 , 2*energy) ;
	//	RooRealVar sigmaG("sigmaG" , "sigmaG" , sigma.getVal() , 0 , 2*energy/5) ;
	//	RooGaussian gauss("gauss" , "gauss" , x , meanG , sigmaG) ;
	//	//gauss.fitTo(dh, Range("1.5sigma")) ;
	//	// gauss.fitTo(dh, Range("2sigma")) ;
	//	gauss.fitTo(dh, Range("rightPart")) ;
	//	gauss.plotOn(frame , LineColor(kRed) ) ;
	//	gauss.plotOn(frame, Range("Full") , LineStyle(kDashed) , LineColor(kRed) ) ;
	//	gauss.paramOn(frame, Layout(0.10,0.45,0.90)) ;
	//	frame->getAttText()->SetTextSize(0.03f) ;

	//frame->Draw() ;
	/*
	cout << "         " << "  mean " << " | " << " sigma " << " | " << " resol " << endl ;
	cout << "    CB : " << mean.getVal() << " | " << sigma.getVal() << " | " << 100*sigma.getVal()/mean.getVal() << endl ;
	cout << " GAUSS : " << meanG.getVal() << " | " << sigmaG.getVal() << " | " << 100*sigmaG.getVal()/meanG.getVal() << endl ;
	cout << endl ;
	*/

	//	double meanMean = ( mean.getVal()+meanG.getVal() )/ 2.0 ;
	//	double meanResol = ( 100*( sigma.getVal()/mean.getVal() + sigmaG.getVal()/meanG.getVal() ) ) / 2.0 ;
	//	double errMean = abs( ( mean.getVal()-meanG.getVal() )/ 2.0 ) ;
	//	double errResol = abs( ( 100*( sigma.getVal()/mean.getVal() - sigmaG.getVal()/meanG.getVal() ) ) / 2.0 ) ;

	/*
	cout << "MEAN : " << meanMean << " +- " << errMean << endl ;
	cout << "RESOL : " << meanResol << " +- " << errResol << endl ;
	cout << endl ;
	cout << "LIMIT : " << mean.getVal() - alpha.getVal()*sigma.getVal() << endl ;
	cout << "    % : " << 100*( integralGauss/integralCB ) << endl ;
	*/

	//	fitMap[energy].mean = meanMean ;
	//	fitMap[energy].resol = meanResol ;
	//	fitMap[energy].meanErr = errMean ;
	//	fitMap[energy].resolErr = errResol ;

//	plotMap[energy] = frame ;

}

TGraphErrors* Fit::getLin(bool restrain) const
{
	TGraphErrors* linGraph = new TGraphErrors() ;
	int point = 0 ;
	for( std::map<float,fitResults>::const_iterator it = fitMap.begin() ; it != fitMap.end() ; ++it)
	{
		if (restrain)
			if ( !(int(it->first)%10 == 0 && it->first < 81 ) )
				continue ;

		linGraph->SetPoint(point , it->first , (it->second).mean ) ;
		linGraph->SetPointError(point++ , 0.0 , (it->second).meanErr ) ;
	}
	linGraph->SetMarkerStyle(20) ;
	return linGraph ;
}
void Fit::drawLin()
{
	TGraphErrors* linGraph = getLin() ;
	TCanvas* c = new TCanvas("linearity" , "linearity" ,800,450) ;
	c->cd() ;
	c->SetTicks() ;
	c->SetGrid() ;
	linGraph->Draw("AP") ;
	c->SaveAs("lin.png") ;
}

TGraphErrors* Fit::getLinDev(bool restrain) const
{
	TGraphErrors* linDevGraph = new TGraphErrors() ;
	int point = 0 ;
	for( std::map<float,fitResults>::const_iterator it = fitMap.begin() ; it != fitMap.end() ; ++it)
	{
		if (restrain)
			if ( !(int(it->first)%10 == 0 && it->first < 81 ) )
				continue ;

		double dev = ( (it->second).mean - it->first) / (it->first) ;
		double devError = ( (it->second).meanErr ) / (it->first) ;
		linDevGraph->SetPoint(point , it->first , dev ) ;
		linDevGraph->SetPointError(point++ , 0.0 , devError ) ;
	}
	linDevGraph->SetMarkerStyle(20) ;
	return linDevGraph ;
}
void Fit::drawLinDev()
{
	TGraphErrors* linDevGraph = getLinDev() ;
	TCanvas* c = new TCanvas("linearityDev" , "linearityDev" ,800,450) ;
	c->cd() ;
	c->SetTicks() ;
	c->SetGrid() ;
	linDevGraph->SetMinimum(-0.15) ;
	linDevGraph->SetMaximum(0.15) ;
	linDevGraph->Draw("AP") ;
	c->SaveAs("linDev.png") ;
}

TGraphErrors* Fit::getResol(bool restrain) const
{
	TGraphErrors* resolGraph = new TGraphErrors() ;
	int point = 0 ;
	for( std::map<float,fitResults>::const_iterator it = fitMap.begin() ; it != fitMap.end() ; ++it)
	{
		if (restrain)
			if ( !(int(it->first)%10 == 0 && it->first < 81 ) )
				continue ;

		resolGraph->SetPoint(point , it->first , (it->second).resol ) ;
		resolGraph->SetPointError(point++ , 0.0 , (it->second).resolErr ) ;
	}
	resolGraph->SetMarkerStyle(20) ;
	return resolGraph ;
}

void Fit::drawResol()
{
	TGraphErrors* resolGraph = getResol() ;
	TCanvas* c = new TCanvas("resolution" , "resolution" ,800,450) ;
	c->cd() ;
	c->SetTicks() ;
	c->SetGrid() ;
	resolGraph->Draw("AP") ;
	c->SaveAs("resol.png") ;
}
