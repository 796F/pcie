
module cl_miner () (
  // where do we pass in clk?
  input clk,
  // how does PCI write and read data?
  input reset,
	input [1151:0] block,
	output [31:0] nonce_out
  output nonce_found
);
	
  ////////////////////////////////////////////////////////////////////////////
	// Mining Cores - Set CORES Parameter For The Algo Being Mined
	////////////////////////////////////////////////////////////////////////////
	
	parameter CORES = 2;
	
	wire [(CORES*32)-1:0] nonce_m;
	wire [CORES-1:0] found_m;

  nonce_found = found_m[0];
  nonce_found = found_m[1];

	genvar i;
	generate
		for ( i = 0; i < CORES; i = i + 1 ) begin : MINER

		// Miner Core
		miner # (.CORES(CORES)) m (
			.clk(clk),
			.reset(reset),
			.block(block),
			.nonce_start(i),
			.nonce_out(nonce_m[(i*32)+:32]),
			.nonce_found(found_m[i])
		);

		end
	endgenerate
  
  // on each clock edge
	always @(posedge clk) begin
  // check if found_m is marked true for amy cores.  
  // if true, send results out to nonce_out
		     if ( found_m[ 0] ) nonce_out <= nonce_m[  0 +: 32];
		else if ( found_m[ 1] ) nonce_out <= nonce_m[ 32 +: 32];

// to add more cores, uncomment the below.  
//		else if ( found_m[ 2] ) nonce_out <= nonce_m[ 64 +: 32];
//		else if ( found_m[ 3] ) nonce_out <= nonce_m[ 96 +: 32];

		else nonce_out <= nonce_out;

	end
  
endmodule
