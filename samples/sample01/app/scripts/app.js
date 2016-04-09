var h5asApp = angular.module('h5asApp', ['ngRoute','ngTable']);

h5asApp.config(function ($routeProvider) {
    $routeProvider

        .when('/', {
            templateUrl: 'partials/home.html',
            controller: 'HomeController'
        })
        .when('/cav', {
            templateUrl: 'partials/customer_add_view_tmpl.html',
            controller: 'CAViewController'
        })
       .when('/clv', {
           templateUrl: 'partials/customer_list_view_tmpl.html',
            controller: 'CLViewController'
    });
});


h5asApp.controller('HomeController', function ($scope) {
    $scope.message = 'Home View';

});


h5asApp.controller('CAViewController', function ($scope) {
    $scope.message = 'Customer Add View';
});

h5asApp.controller('CLViewController', function ($scope, $http, $filter, ngTableParams) {
    $scope.message = 'Customer List View';

    $scope.loading = true;
    $scope.addMode = false;

    //Used to display the data
    $http.get('/api/customers').success(function (data) {
        $scope.customers = data;
        $scope.loading = false;
        console.log('been here');
        $scope.tableParams = new ngTableParams({
            page: 1,
            count: 10
        },{
            total: $scope.customers.length,
            data: $scope.customers,
            getData: function($defer, params) {
                var orderedData = params.sorting() ? $filter('orderBy')($scope.customers, params.orderBy()) : $scope.customers;
                $defer.resolve(orderedData.slice((params.page() - 1) * params.count(), params.page() * params.count()));
            }
        });

        $scope.editId = -1;

        $scope.setEditId =  function(pid) {
            $scope.editId = pid;
        }
    })
    .error(function () {
        $scope.error = "An Error has occured while loading posts!";
        $scope.loading = false;
    });
    
   

    $scope.toggleEdit = function () {
        this.customer.editMode = !this.customer.editMode;
    };
    $scope.toggleAdd = function () {
        $scope.addMode = !$scope.addMode;
    };

    //Used to save a record after edit
    $scope.save = function () {
        $scope.loading = true;
        var cust = this.customer;
        console.log(cust);
        $http.put('/api/customers', cust).success(function (data) {
            alert("Saved Successfully!!");
            cust.editMode = false;
            $scope.loading = false;
        }).error(function (data) {
            $scope.error = "An Error has occured while Saving Friend! " + data;
            $scope.loading = false;

        });
    };

    //Used to add a new record
    $scope.add = function () {
        $scope.loading = true;
        $http.post('/api/customers', this.newfriend).success(function (data) {
            alert("Added Successfully!!");
            $scope.addMode = false;
            $scope.friends.push(data);
            $scope.loading = false;
        }).error(function (data) {
            $scope.error = "An Error has occured while Adding Friend! " + data;
            $scope.loading = false;

        });
    };

    //Used to edit a record
    $scope.deletefriend = function () {
        $scope.loading = true;
        var customerid = this.customer.customer_id;
        $http.delete('/api/customers?ID=' + customerid).success(function (data) {
            alert("Deleted Successfully!!");
            $.each($scope.customers, function (i) {
                if ($scope.customers[i].customer_id === customerid) {
                    $scope.customers.splice(i, 1);
                    return false;
                }
            });
            $scope.loading = false;
        }).error(function (data) {
            $scope.error = "An Error has occured while Saving Friend! " + data;
            $scope.loading = false;

        });
    };

});


