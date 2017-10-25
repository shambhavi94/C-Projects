#include <iostream>
#include <string>
 using namespace std;

	float similarityScore(string sequenceOne, string sequenceTwo){
    int i = 0;
	float matchingchars = 0;    
	float hemDist = 0;

	float seqOneLength = sequenceOne.length();

	if (seqOneLength != sequenceTwo.length()){
        return 0;
    }

    else {
        for (i = 0; i < seqOneLength;i++){

            if(sequenceOne[i] == sequenceTwo[i]){
                matchingchars++;
            }
            else {
                hemDist++;
            }

        }
     }
    return (seqOneLength-hemDist)/seqOneLength;

}


int countMatches(string genome, string sequence1, float min_Score) {
   int match = 0;
   int sequence1Len = sequence1.length();

   int i = 0;
   while (i <= genome.length() - sequence1Len) {
     float sScore = similarityScore(genome.substr(i, sequence1Len), sequence1);
     if (sScore >= min_Score) {
       match++;
     }
     i++;
   }

   return match;
 }



float findBestMatch(string genome, string seq) {
   float bestMatch = 0.0;

   int i = 0;
   while (i < genome.length()) {
     float sScore = similarityScore(genome.substr(i, seq.length()), seq);
     if (sScore > bestMatch) {
       bestMatch = sScore;
     }
     i++;
   }

   return bestMatch;
 }


 int findBestGenome(string genome1, string genome2, string genome3, string seq) {
   float human_genome = findBestMatch(genome1, seq);
   float mouse_genome = findBestMatch(genome2, seq);
   float unknown_genome = findBestMatch(genome3, seq);

   if (human_genome > mouse_genome && human_genome > unknown_genome) {
     return 1;
   } else if (mouse_genome > human_genome && mouse_genome > unknown_genome) {
     return 2;
   } else if (unknown_genome > human_genome && unknown_genome > mouse_genome) {
     return 3;
   } else {
     return 0;
   }
 }

/*int main() {
   string humanDNA = "CGCAAATTTGCCGGATTTCCTTTGCTGTTCCTGCATGTAGTTTAAACGAGATTGCCAGCACCGGGTATCATTCACCATTTTTCTTTTCGTTAACTTGCCGTCAGCCTTTTCTTTGACCTCTTCTTTCTGTTCATGTGTATTTGCTGTCTCTTAGCCCAGACTTCCCGTGTCCTTTCCACCGGGCCTTTGAGAGGTCACAGGGTCTTGATGCTGTGGTCTTCATCTGCAGGTGTCTGACTTCCAGCAACTGCTGGCCTGTGCCAGGGTGCAGCTGAGCACTGGAGTGGAGTTTTCCTGTGGAGAGGAGCCATGCCTAGAGTGGGATGGGCCATTGTTCATG";
   string mouseDNA = "CGCAATTTTTACTTAATTCTTTTTCTTTTAATTCATATATTTTTAATATGTTTACTATTAATGGTTATCATTCACCATTTAACTATTTGTTATTTTGACGTCATTTTTTTCTATTTCCTCTTTTTTCAATTCATGTTTATTTTCTGTATTTTTGTTAAGTTTTCACAAGTCTAATATAATTGTCCTTTGAGAGGTTATTTGGTCTATATTTTTTTTTCTTCATCTGTATTTTTATGATTTCATTTAATTGATTTTCATTGACAGGGTTCTGCTGTGTTCTGGATTGTATTTTTCTTGTGGAGAGGAACTATTTCTTGAGTGGGATGTACCTTTGTTCTTG";
   string unkownDNA = "CGCATTTTTGCCGGTTTTCCTTTGCTGTTTATTCATTTATTTTAAACGATATTTATATCATCGGGTTTCATTCACTATTTTTCTTTTCGATAAATTTTTGTCAGCATTTTCTTTTACCTCTTCTTTCTGTTTATGTTAATTTTCTGTTTCTTAACCCAGTCTTCTCGATTCTTATCTACCGGACCTATTATAGGTCACAGGGTCTTGATGCTTTGGTTTTCATCTGCAAGAGTCTGACTTCCTGCTAATGCTGTTCTGTGTCAGGGTGCATCTGAGCACTGATGTGGAGTTTTCTTGTGGATATGAGCCATTCATAGTGTGGGATGTGCCATAGTTCATG";

   cout << similarityScore(humanDNA, mouseDNA) << endl;
   cout << countMatches(humanDNA, "CGC", 0.60) << endl;
   cout << findBestMatch(mouseDNA, "ACT") << endl;
   cout << findBestGenome("GGAACACA", "CGATATGA", "GGAGTA", "CAATC") << endl;

 }*/


