function CompressUCL()
{
  ucl = new QtUCL() ;
  ucl . ToLzo ( "Testing.txt" , "Testing.ucl" , 9 , 1 ) ;
  delete ucl ;
}

function DecompressUCL()
{
  ucl = new QtUCL() ;
  ucl . ToFile ( "Testing.ucl" , "Testing.txt" ) ;
  delete ucl ;
}
