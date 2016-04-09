
 
		 
function customerController($scope, $http) {
$http.get('/api/customers').success(function (data) {
        $scope.customer = data;
    })
   .error(function () {
       $scope.error = "An Error has occured while loading posts!";
       $scope.loading = false;
   });
}
		 

	 
	 