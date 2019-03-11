/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

import java.io.*;
import java.math.*;
import java.util.*;

class attack {
  private Process        target     = null;

  private BufferedReader target_out = null;
  private PrintWriter    target_in  = null;

  public attack( String filename ) throws IOException {
    // Produce a sub-process representing the attack target.

    this.target     = Runtime.getRuntime().exec( filename );

    // Construct handles to attack target standard input and output.

    this.target_out = new BufferedReader( new InputStreamReader( target.getInputStream() ) );
    this.target_in  = new PrintWriter( target.getOutputStream(), true );
  }

  public int[] interact( String G ) throws IOException {
    // Send      G      to   attack target.

    this.target_in.println( G ); this.target_in.flush();

    // Receive ( t, r ) from attack target.

    Integer t = new Integer( this.target_out.readLine() );
    Integer r = new Integer( this.target_out.readLine() );

    return new int[] { t, r };
  }

  public void attack() throws IOException {
    // Select a hard-coded guess ...

    String G = "guess";

    // ... then interact with the attack target.

    int[] output = interact( G );

    int t = output[ 0 ];
    int r = output[ 1 ];

    // Print all of the inputs and outputs.

    System.out.println( "G = " +                   G   );
    System.out.println( "t = " + Integer.toString( t ) );
    System.out.println( "r = " + Integer.toString( r ) );
  }

  public static void main( String[] argv ) {
    try {
      // Execute a function representing the attacker.

      ( new attack( argv[ 0 ] ) ).attack();
    }
    catch( Exception e ) {
      e.printStackTrace();
    }
  }
}
