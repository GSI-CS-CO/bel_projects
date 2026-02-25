// Function to build a single gateware target on a specific branch
def buildGateware(gateware, branchName) {
   // Determine Quartus installation path based on version
   def quartus = "/opt/quartus/${gateware.quartus}/quartus"

   // Use dedicated node labeled 'quartus'
   node('quartus') {
       // Skip default SCM checkout (we handle it manually)
       skipDefaultCheckout()

       // Use a separate workspace for each branch + gateware combination
       ws("workspace/${JOB_NAME}/${branchName}/${gateware.name}") {
           withEnv([
               "QUARTUS=${quartus}/",
               "QUARTUS_ROOTDIR=${quartus}",
               "QSYS_ROOTDIR=${quartus}/sopc_builder/bin",
               "QUARTUS_64BIT=1",
               // Extend PATH and LD_LIBRARY_PATH for Quartus and project scripts
               "PATH=${PATH}:${quartus}/sopc_builder/bin:${HOME}/.local/bin:${WORKSPACE}/res/rocky-9",
               "LD_LIBRARY_PATH=${WORKSPACE}/res/rocky-9",
           ]) {

               // -----------------------------
               // Stage: Checkout the repository
               // -----------------------------
               stage("checkout ${gateware.name}") {
                   // Clean workspace for a fresh clone
                   deleteDir()

                   checkout scmGit(
                       branches: [[name: "*/${branchName}"]],
                       extensions: [[$class: 'SubmoduleOption', parentCredentials: true]],
                       userRemoteConfigs: [[url: "${GIT_UPSTREAM}"]]
                   )
               }

               // -----------------------------
               // Stage: Prepare build environment
               // -----------------------------
               stage("prepare ${gateware.name}") {
                   sh "cd res/rocky-9 && ./generate_soft_links.sh"
                   sh "git config --global user.name \"Timing Group Jenkins\""
                   sh "git config --global user.email \"tos-service@gsi.de\""
               }

               // -----------------------------
               // Stage: Build the gateware
               // -----------------------------
               stage("build ${gateware.name}") {
                   wrap([$class: 'Xvnc', takeScreenshot: false, useXauthority: true]) {
                       sh "make"
                       sh "make ${gateware.name} BUILD_ARTIFACT=yes"
                       // Only run -check if quartus is not "none"
                       if (gateware.quartus != "none") {
                           sh "make ${gateware.name}-check BUILD_ARTIFACT=yes"
                       }
                   }
               }

               // -----------------------------
               // Stage: Archive build artifacts
               // -----------------------------
               stage("archive ${gateware.name}") {
                   archiveArtifacts artifacts: """
                       **/artifacts/gw-*.rpd,
                       **/artifacts/gw-*.sof,
                       **/artifacts/gw-*.jic,
                       **/artifacts/fw-fg-scu2-scu3-*.bin,
                       **/artifacts/fw-bg-pexp-pmc-amc-pex5-exp5-*.bin,
                   """, allowEmptyArchive: true
               }
           }
       }
   }
}

// =============================
// Main Pipeline Definition
// =============================
pipeline {
   agent none // Nodes are assigned per gateware stage
   options {
       // Keep last 7 builds and last 2 artifacts
       buildDiscarder(logRotator(numToKeepStr: '7', artifactNumToKeepStr: '7'))
   }
   triggers {
       // Poll the repository every 15 minutes (no webhook needed)
       pollSCM('H/15 * * * *')
       // Daily build between 20:00 and 23:00
       cron('H H(20-23) * * *')
   }
   environment {
       // Git repository upstream URL
       GIT_UPSTREAM = 'https://github.com/GSI-CS-CO/bel_projects.git'
   }
   stages {
       stage('Gateware Build') {
           steps {
               script {

                   // Use the branch Jenkins is currently building
                   def branchName = env.BRANCH_NAME

                   // Define the gateware list including firmware
                   def gatewares = [
                       [name: "scu2", quartus: "18"],
                       [name: "scu3", quartus: "18"],
                       [name: "vetar2a", quartus: "18"],
                       [name: "vetar2a-ee-butis", quartus: "18"],
                       [name: "pexarria5", quartus: "18"],
                       [name: "pexarria5-sdr", quartus: "18"],
                       [name: "ftm", quartus: "18"],
                       [name: "exploder5", quartus: "18"],
                       [name: "pmc", quartus: "18"],
                       [name: "microtca", quartus: "18"],
                       [name: "pexp", quartus: "18"],
                       [name: "pexp-sdr", quartus: "18"],
                       [name: "pexp-pps", quartus: "18"],
                       [name: "pexp-neorv32", quartus: "18"],
                       [name: "pexarria10", quartus: "23"],
                       [name: "ftm10", quartus: "23"],
                       [name: "scu4slim", quartus: "23"],
                       [name: "scu5", quartus: "23"],
                       [name: "ftm5dp", quartus: "23"],
                       [name: "fw-bg", quartus: "none"],
                       [name: "fw-fg-scu2-scu3", quartus: "none"]
                   ]

                   // Create parallel jobs per gateware
                   def gatewareJobs = gatewares.collectEntries { gw ->
                       ["${gw.name}": { buildGateware(gw, branchName) }]
                   }

                   // Run all gateware builds in parallel
                   parallel gatewareJobs
               }
           }
       }
   }
   post {
       always {
           echo "All gateware builds finished."
       }
       failure {
           echo "Some builds failed."
       }
       success {
           echo "All builds succeeded!"
       }
   }
}
