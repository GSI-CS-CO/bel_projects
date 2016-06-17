module altera_lut (
  lut_i,
  lut_o
);

  input  wire lut_i;
  output wire lut_o;
  
  // https://www.altera.com/support/support-resources/knowledge-base/solutions/rd10132014_244.html
  parameter lut_mask = 64'hAAAAAAAAAAAAAAAA; 
  parameter dont_touch = "on";

  arriav_lcell_comb wirelut( .dataa(lut_i), .combout(lut_o) );
  defparam wirelut.lut_mask = 64'hAAAAAAAAAAAAAAAA ;
  defparam wirelut.dont_touch = "on";
  
endmodule
